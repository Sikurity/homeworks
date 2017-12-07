import java.sql.ResultSet;
import java.sql.Timestamp;
import java.sql.Types;
import java.util.ArrayList;

public class LoginDTO extends DataTransferObject
{
	private String 		student_id;
	private String 		password;
	private String 		name;
	private String 		sex;
	private String 		major_id;
	private String 		tutor_id;
	private String 		year;
	private Timestamp	lastlogin;
	
	static
	{
		fields.put(LoginDTO.class.getName(), new ArrayList<Integer>());
		fields.get(LoginDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LoginDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LoginDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LoginDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LoginDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LoginDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LoginDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LoginDTO.class.getName()).add(new Integer(Types.TIMESTAMP));
	}
	
	public LoginDTO(String student_id, String password, String name, String sex, String major_id, String tutor_id, String year, Timestamp lastlogin) 
	{
		super();
		this.student_id = student_id;
		this.password 	= password;
		this.name 		= name;
		this.sex 		= sex;
		this.major_id 	= major_id;
		this.tutor_id 	= tutor_id;
		this.year 		= year;
		this.lastlogin 	= lastlogin;
	}

	public LoginDTO()
	{
		this(null, null, null, null, null, null, null, null);
	}

	public static Object getInstance() 
	{
		// TODO Auto-generated method stub
		return (Object)(new LoginDTO());
	}

	public String getStudent_id() {
		return student_id;
	}

	public void setStudent_id(String student_id) {
		this.student_id = student_id;
	}

	public String getPassword() {
		return password;
	}

	public void setPassword(String password) {
		this.password = password;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getSex() {
		return sex;
	}

	public void setSex(String sex) {
		this.sex = sex;
	}

	public String getMajor_id() {
		return major_id;
	}

	public void setMajor_id(String major_id) {
		this.major_id = major_id;
	}

	public String getTutor_id() {
		return tutor_id;
	}

	public void setTutor_id(String tutor_id) {
		this.tutor_id = tutor_id;
	}

	public String getYear() {
		return year;
	}

	public void setYear(String year) {
		this.year = year;
	}

	public Timestamp getLastlogin() {
		return lastlogin;
	}

	public void setLastlogin(Timestamp lastlogin) {
		this.lastlogin = lastlogin;
	}

	@Override
	public void setValues(ResultSet rs) 
	{
		student_id	= ((String)getColumnValueFromResultSet(LoginDTO.class.getName(), rs, 0, "student_id"));
		password	= ((String)getColumnValueFromResultSet(LoginDTO.class.getName(), rs, 1, "password"));
		name		= ((String)getColumnValueFromResultSet(LoginDTO.class.getName(), rs, 2, "name"));
		sex			= ((String)getColumnValueFromResultSet(LoginDTO.class.getName(), rs, 3, "sex"));
		major_id	= ((String)getColumnValueFromResultSet(LoginDTO.class.getName(), rs, 4, "major_id"));
		tutor_id	= ((String)getColumnValueFromResultSet(LoginDTO.class.getName(), rs, 5, "tutor_id"));
		year		= ((String)getColumnValueFromResultSet(LoginDTO.class.getName(), rs, 6, "year"));
		lastlogin	= ((Timestamp)getColumnValueFromResultSet(LoginDTO.class.getName(), rs, 7, "lastlogin"));
	}
}
