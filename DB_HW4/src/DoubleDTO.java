import java.sql.ResultSet;
import java.sql.Types;
import java.util.ArrayList;

public class DoubleDTO extends DataTransferObject
{
	private double number;
	
	static
	{
		fields.put(DoubleDTO.class.getName(), new ArrayList<Integer>());
		fields.get(DoubleDTO.class.getName()).add(new Integer(Types.DOUBLE));
	}
	
	public DoubleDTO(double number) 
	{
		super();
		this.number = number;
	}

	public DoubleDTO()
	{
		this(0.0);
	}

	public static Object getInstance() 
	{
		// TODO Auto-generated method stub
		return (Object)(new DoubleDTO());
	}

	public double getNumber() {
		return number;
	}

	public void setNumber(double number) {
		this.number = number;
	}

	@Override
	public void setValues(ResultSet rs) 
	{
		number	= ((double)getColumnValueFromResultSet(DoubleDTO.class.getName(), rs, 0, "number"));
	}
}
