import java.sql.ResultSet;
import java.sql.Types;
import java.util.ArrayList;

public class LectureDTO extends DataTransferObject
{
	private String 		class_id;
	private String 		class_no;
	private String 		course_id;
	private String 		name;
	private String 		major_id;
	private String 		year;
	private int			credit;
	private String 		lecturer_id;
	private int			person_max;
	private String 		opened;
	private String 		room_id;
	private String 		lecturer_name;
	private String 		major_name;
	private String 		building_name;
	private String		begin;
	private String		end;
	
	static
	{
		fields.put(LectureDTO.class.getName(), new ArrayList<Integer>());
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.INTEGER));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.INTEGER));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
		fields.get(LectureDTO.class.getName()).add(new Integer(Types.VARCHAR));
	}
	
	public LectureDTO(String class_id, String class_no, String course_id, String name, String major_id, String year, int credit, String lecturer_id, 
			int person_max, String opened, String room_id, String lecturer_name, String major_name, String building_name, String begin, String end) 
	{
		super();
		this.class_id 		= class_id;
		this.class_no 		= class_no;
		this.course_id 		= course_id;
		this.name 			= name;
		this.major_id 		= major_id;
		this.year 			= year;
		this.credit 		= credit;
		this.lecturer_id 	= lecturer_id;
		this.person_max 	= person_max;
		this.opened 		= opened;
		this.room_id 		= room_id;
		this.lecturer_name 	= lecturer_name; 
		this.major_name		= major_name;
		this.building_name	= building_name;
		this.begin			= begin;
		this.end			= end;
	}
	
	public LectureDTO()
	{
		this(null, null, null, null, null, null, 0, null, 0, null, null, null, null, null, null, null);
	}

	public String getClass_id() {
		return class_id;
	}

	public void setClass_id(String class_id) {
		this.class_id = class_id;
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

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getMajor_id() {
		return major_id;
	}

	public void setMajor_id(String major_id) {
		this.major_id = major_id;
	}

	public String getYear() {
		return year;
	}

	public void setYear(String year) {
		this.year = year;
	}

	public int getCredit() {
		return credit;
	}

	public void setCredit(int credit) {
		this.credit = credit;
	}

	public String getLecturer_id() {
		return lecturer_id;
	}

	public void setLecturer_id(String lecturer_id) {
		this.lecturer_id = lecturer_id;
	}

	public int getPerson_max() {
		return person_max;
	}

	public void setPerson_max(int person_max) {
		this.person_max = person_max;
	}

	public String getOpened() {
		return opened;
	}

	public void setOpened(String opened) {
		this.opened = opened;
	}

	public String getRoom_id() {
		return room_id;
	}

	public void setRoom_id(String room_id) {
		this.room_id = room_id;
	}

	public String getLecturer_name() {
		return lecturer_name;
	}

	public void setLecturer_name(String lecturer_name) {
		this.lecturer_name = lecturer_name;
	}

	public String getMajor_name() {
		return major_name;
	}

	public void setMajor_name(String major_name) {
		this.major_name = major_name;
	}

	public String getBuilding_name() {
		return building_name;
	}

	public void setBuilding_name(String building_name) {
		this.building_name = building_name;
	}

	public String getBegin() {
		return begin;
	}

	public void setBegin(String begin) {
		this.begin = begin;
	}

	public String getEnd() {
		return end;
	}

	public void setEnd(String end) {
		this.end = end;
	}
	
	@Override
	public void setValues(ResultSet rs) 
	{
		class_id		= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 0, "class_id"));
		class_no		= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 1, "class_no"));
		course_id		= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 2, "course_id"));
		name			= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 3, "name"));
		major_id		= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 4, "major_id"));
		year			= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 5, "year"));
		credit			= ((Integer)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 6, "credit"));
		lecturer_id		= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 7, "lecturer_id"));
		person_max		= ((Integer)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 8, "person_max"));
		opened			= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 9, "opened"));
		room_id			= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 10, "room_id"));
		lecturer_name	= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 11, "lecturer_name"));
		major_name		= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 12, "major_name"));
		building_name	= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 13, "building_name"));
		begin			= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 14, "begin"));
		end				= ((String)getColumnValueFromResultSet(LectureDTO.class.getName(), rs, 15, "end"));
	}
}
