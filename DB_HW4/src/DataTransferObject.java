import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Types;
import java.util.ArrayList;
import java.util.Map;
import java.util.TreeMap;

public abstract class DataTransferObject
{
	public static Map<String, ArrayList<Integer>> fields = new TreeMap<String, ArrayList<Integer>>();
	
	public DataTransferObject()
	{
		
	}
	
	public static ArrayList<Integer> getFields(String className)
	{
		return fields.get(className);
	}
	
	public abstract void setValues(ResultSet rs);

	public Object getColumnValueFromResultSet(String className, ResultSet rs, int fieldNum, String fieldName) 
	{
		Object result;
		
		try
		{
			switch( fields.get(className).get(fieldNum) )
			{
			case Types.VARCHAR:
				result = rs.getString(fieldName);
				break;
			case Types.INTEGER:
				result = rs.getInt(fieldName);
				break;
			case Types.BIGINT:
				result = rs.getBigDecimal(fieldName);
				break;
			case Types.DOUBLE:
				result = rs.getDouble(fieldName);
				break;
			case Types.TIMESTAMP:
				result = rs.getTimestamp(fieldName);
				break;
			default:
				result = null;
				break;
			}
			
			return result;
		}
		catch(SQLException sqlEx)
		{
			sqlEx.printStackTrace();
			return null;
		}
	}
}
