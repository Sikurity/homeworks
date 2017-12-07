import java.math.BigDecimal;
import java.sql.ResultSet;
import java.sql.Types;
import java.util.ArrayList;

public class BigintDTO extends DataTransferObject
{
	private BigDecimal number;
	
	static
	{
		fields.put(BigintDTO.class.getName(), new ArrayList<Integer>());
		fields.get(BigintDTO.class.getName()).add(new Integer(Types.BIGINT));
	}
	
	public BigintDTO(BigDecimal number) 
	{
		super();
		this.number = number;
	}

	public BigintDTO()
	{
		this(new BigDecimal("0"));
	}

	public static Object getInstance() 
	{
		// TODO Auto-generated method stub
		return (Object)(new BigintDTO());
	}

	public BigDecimal getNumber() {
		return number;
	}

	public void setNumber(BigDecimal number) {
		this.number = number;
	}

	@Override
	public void setValues(ResultSet rs) 
	{
		number	= ((BigDecimal)getColumnValueFromResultSet(BigintDTO.class.getName(), rs, 0, "number"));
	}
}
