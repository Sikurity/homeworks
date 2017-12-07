import java.sql.ResultSet;
import java.sql.Timestamp;
import java.sql.Types;
import java.util.ArrayList;

public class EnrollmentDTO extends DataTransferObject
{
	private String 		class_no;
	private String 		course_id;
	private String 		student_id;
	private int			wished;
	private int 		enrolled;
	private int			lastwished;
	private int 		lastenrolled;
	private Timestamp	lastupdated;
	private String 		opened;
	private String 		achievement;
	
	static
	{
		fields.put(EnrollmentDTO.class.getName(), new ArrayList<Integer>());
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.INTEGER));
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.INTEGER));
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.INTEGER));
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.INTEGER));
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.TIMESTAMP));
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(EnrollmentDTO.class.getName()).add(new Integer(Types.VARCHAR));
	}

	public EnrollmentDTO(String class_no, String course_id, String student_id, int wished, int enrolled, int lastwished, int lastenrolled,
			Timestamp lastupdated, String opened, String achievement)
	{
		super();
		this.class_no = class_no;
		this.course_id = course_id;
		this.student_id = student_id;
		this.wished = wished;
		this.enrolled = enrolled;
		this.lastwished = lastwished;
		this.lastenrolled = lastenrolled;
		this.lastupdated = lastupdated;
		this.opened = opened;
		this.achievement = achievement;
	}

	public EnrollmentDTO()
	{
		this(null, null, null, 0, 0, 0, 0, null, null, null);
	}

	public static Object getInstance() 
	{
		// TODO Auto-generated method stub
		return (Object)(new EnrollmentDTO());
	}
	
	public String getClass_no() {
		return class_no;
	}

	public void setClass_no(String class_no) {
		this.class_no = class_no;
	}
	
	public String getCourse_id() {
		return course_id;
	}

	public void setCourse_id(String course_id) {
		this.course_id = course_id;
	}

	public String getStudent_id() {
		return student_id;
	}

	public void setStudent_id(String student_id) {
		this.student_id = student_id;
	}

	public int getWished() {
		return wished;
	}

	public void setWished(int wished) {
		this.wished = wished;
	}

	public int getEnrolled() {
		return enrolled;
	}

	public void setEnrolled(int enrolled) {
		this.enrolled = enrolled;
	}

	public int getLastwished() {
		return lastwished;
	}

	public void setLastwished(int lastwished) {
		this.lastwished = lastwished;
	}

	public int getLastenrolled() {
		return lastenrolled;
	}

	public void setLastenrolled(int lastenrolled) {
		this.lastenrolled = lastenrolled;
	}

	public Timestamp getLastupdated() {
		return lastupdated;
	}

	public void setLastupdated(Timestamp lastupdated) {
		this.lastupdated = lastupdated;
	}

	public String getOpened() {
		return opened;
	}

	public void setOpened(String opened) {
		this.opened = opened;
	}

	public String getAchievement() {
		return achievement;
	}

	public void setAchievement(String achievement) {
		this.achievement = achievement;
	}

	@Override
	public void setValues(ResultSet rs) 
	{
		class_no		= ((String)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 0, "class_no"));
		course_id		= ((String)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 1, "course_id"));
		student_id		= ((String)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 2, "student_id"));
		wished			= ((Integer)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 3, "wished"));
		enrolled		= ((Integer)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 4, "enrolled"));
		lastwished		= ((Integer)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 5, "lastwished"));
		lastenrolled	= ((Integer)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 6, "lastenrolled"));
		lastupdated		= ((Timestamp)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 7, "lastupdated"));
		opened			= ((String)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 8, "opened"));
		achievement		= ((String)getColumnValueFromResultSet(EnrollmentDTO.class.getName(), rs, 9, "achievement"));
	}
}
