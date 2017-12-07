import java.sql.*;
import java.util.ArrayList;

public class DBManager
{
	private ResultSet rs;
	private Statement stmt;
	private PreparedStatement pstmt;
	private Connection conn;
	
	private final String uri = "jdbc:mysql://localhost:3306/dblec?useUnicode=true&characterEncoding=utf8&autoReconnect=true&useSSL=false";
	private final String id = "DB_ID";
	private final String pw = "DB_PASSWORD";

	private static DBManager singletone = new DBManager();
	
	private DBManager()
	{
		
	}
	
	public static DBManager getInstance()
	{
		if( singletone == null)
			singletone = new DBManager();
		
		return singletone;
	}
	
	private boolean connect()
	{
		try
		{
			conn = DriverManager.getConnection(uri, id, pw);
			
			return conn != null;
		}
		catch(SQLException e)
		{
			System.out.println("DB 연결실패");
			e.printStackTrace();
			
			return false;
		}
	}
	
	private void disconnect()
	{
		try
		{
			if(rs != null)
				rs.close();
			
			if( stmt != null )
				stmt.close();
			
			if( pstmt != null )
				pstmt.close();
			
			if( conn != null )
				conn.close();
		}
		catch(SQLException e)
		{
			System.out.println("DB 연결 해제 중 에러 발생");
			e.printStackTrace();
		}
	}

	private <T extends DataTransferObject> boolean validateFields(Class<T> template)
	{
		int i, cols, size;
		ResultSetMetaData rsMetaData;
		
		ArrayList<Integer> fields;
		
		try
		{
			rsMetaData= rs.getMetaData();
			
			cols = rsMetaData.getColumnCount();
			
			fields = T.getFields(template.getName());
			size = fields.size();
			if( cols == size )
			{
				for( i = 1 ; i <= size; i++ )
				{
					if( rsMetaData.getColumnType(i) != fields.get(i - 1) )
						return false;
				}
					
				return true;
			}
			else
				return false;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}
	
	public <T extends DataTransferObject> ArrayList<T> selectStaticQuery(Class<T> template, String sql)
	{	    
		ArrayList<T> ret = new ArrayList<T>();
		T element;
		
		try
		{
			if( connect() )
			{
				stmt = conn.createStatement();				
				rs = stmt.executeQuery(sql);
				
				element = template.newInstance();
				if( validateFields(template) )
				{
					while( rs.next() )
					{
						element = template.newInstance();
						element.setValues(rs);
						ret.add(element);
					}
				}
			}
			
			return ret;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
		finally
		{
			 disconnect();
		}
	}
	
	private boolean setItem(int nth, int type, Object item) 
	{
		try
		{
			switch( type )
			{
			case Types.VARCHAR:
				pstmt.setString(nth + 1, (String)item);
				break;
			case Types.INTEGER:
				pstmt.setInt(nth + 1, (Integer)item);
				break;
			case Types.TIMESTAMP:
				pstmt.setTimestamp(nth + 1, (Timestamp)item);
				break;
			default:
				pstmt.setNull(nth + 1, type);
				break;
			}
			
			return true;
		}
		catch(SQLException sqlEx)
		{
			sqlEx.printStackTrace();
			return false;
		}
	}
	
	private boolean applyItemsToSQL(String sql, ArrayList<Integer> types, ArrayList<Object> items)
	{
		int i, blankCnt, itemsCnt;
		
		blankCnt = 0;
		for( i = 0 ; i < sql.length() ; i++ )
			if( sql.charAt(i) == '?' )
				blankCnt++;
				
		itemsCnt = items.size();
		
		if( blankCnt != itemsCnt || itemsCnt != types.size() )
			return false;
		
		for( i = 0 ; i < blankCnt ; i++ )
			setItem(i, types.get(i), items.get(i));
		
		return true;
	}
	
	public <T extends DataTransferObject> ArrayList<T> selectDynamicQuery(Class<T> template, String sql, ArrayList<Integer> types, ArrayList<Object> items)
	{	    
		ArrayList<T> ret = new ArrayList<T>();
		T element;
		
		try
		{
			if( connect() )
			{
				pstmt = conn.prepareStatement(sql);
				applyItemsToSQL(sql, types, items);
				rs = pstmt.executeQuery();

				template.newInstance();
				if( validateFields(template) )
				{
					while( rs.next() )
					{
						element = template.newInstance();
						element.setValues(rs);
						ret.add(element);
					}
				}
			}
			
			return ret;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
		finally
		{
			 disconnect();
		}
	}
	
	public boolean updateStaticQuery(String sql)
	{
		int result = 0;
		
		try
		{
			if( connect() )
			{
				stmt = conn.createStatement();
				
				result = stmt.executeUpdate(sql);	// updateQuery �뜝�룞�삕 �뜝�떎�눦�삕 �뜝�룞�삕�솚, insert - �뜝�룞�삕�뜝�룞�삕 row �뜝�룞�삕�뜝�룞�삕, update - �뜝�룞�삕�뜝�룞�삕 row �뜝�룞�삕�뜝�룞�삕
			}
			
			return result > 0;
		}
		catch(SQLException e)
		{
			e.printStackTrace();
			return false;
		}
		finally
		{
			disconnect();
		}
	}
	
	public boolean updateDynamicQuery(String sql, ArrayList<Integer> types, ArrayList<Object> items)
	{	    
		int result = 0;
		
		try
		{
			if( connect() )
			{
				pstmt = conn.prepareStatement(sql);
				applyItemsToSQL(sql, types, items);
				result = pstmt.executeUpdate();
			}

			return result > 0;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		finally
		{
			 disconnect();
		}
	}
}