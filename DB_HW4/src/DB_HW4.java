import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.Scanner;
import java.sql.Timestamp;
import java.sql.Types;

public class DB_HW4 
{		
	/********************************************************/
	private final static String EXIT 		= "/exit";		// 종료
	private final static String HELP 		= "/help";		// 도움말 표시
	private final static String LOGIN 		= "/login";		// 로그인
	private final static String LOGOUT 		= "/logout";	// 로그아웃
	private final static String ROLLBACK 	= "/rollback";	// 로그인 직전으로 수강신청 롤백
	private final static String SEARCH 		= "/search";	// 수강편람 검색
	private final static String REGIST 		= "/regist";	// 희망 수강신청
	private final static String CANCEL 		= "/cancel";	// 희망 수강신청 취소
	private final static String ENROLL 		= "/enroll";	// 수강신청 
	private final static String WITHDRAW 	= "/withdraw";	// 수강신청 취소
	private final static String WISHLIST 	= "/wishlist";	// 희망수강 목록출력
	private final static String TIMETABLE 	= "/timetable";	// 시간표 출력
	private final static String OPEN		= "/open";		// 강좌개설
	private final static String CLOSE 		= "/close";		// 강좌폐지
	/********************************************************/
	
	/********************************************************/
	private final static String ADMIN_ID 	= "admin";		// 관리자 ID
	private final static String ADMIN_PWD 	= "admin";		// 관리자 Password
	/********************************************************/

	private static boolean 		isAdmin		= false;
	private static DBManager 	dbmanager	= DBManager.getInstance();
	
	private static Scanner sc = new Scanner(System.in);
	private static String inputMsg = "";
	
	private static LoginDTO user = null;
	
	public static void main(String args[])
	{
		while( true )
		{
			try
			{
				System.out.print("명령어 입력(/help) : ");
				inputMsg = sc.nextLine();
				
				if( inputMsg.split(" ")[0].equals(EXIT))
					break;
				else if( inputMsg.split(" ")[0].equals(HELP) )
				{
					System.out.println(EXIT 		+ "\t\t:종료");
					System.out.println(HELP 		+ "\t\t:명령어 사용법 출력");
					System.out.println(LOGIN 		+ "\t\t:로그인");
					System.out.println("/******** 로그인시 사용가능한 기능들 ********/");
					System.out.println(LOGOUT 		+ "\t\t:로그아웃");
					System.out.println(ROLLBACK 	+ "\t:로그인 직전 상태로 되돌리기");
					System.out.println(SEARCH 		+ " -[mode=|name(강의명)|clsn(강의번호)|crsn(학수번호)|prof(교강사명)]");
					System.out.println(SEARCH 		+ "\t\t:수강편람 검색(mode 인자 없을 시 지정과목 출력)");
					System.out.println(REGIST 		+ "\t\t:수강 희망등록");
					System.out.println(CANCEL 		+ "\t\t:수강 희망취소");
					System.out.println(ENROLL 		+ "\t\t:수강 신청");
					System.out.println(WITHDRAW 	+ "\t:수강철회");
					System.out.println(WISHLIST 	+ "\t:수강희망 목록출력");
					System.out.println(TIMETABLE 	+ "\t:수강목록 및 시간표 출력");
					System.out.println("/******** 관리자만 사용가능한 기능들 ********/");
					System.out.println(OPEN 		+ "\t\t- 강좌개설");
					System.out.println(CLOSE 		+ "\t\t- 강좌폐지");
				}
				else if( inputMsg.split(" ")[0].equals(LOGIN))
				{ 
					if( login() )
						System.out.println("로그인 성공 - " + user.getName());
					else
						System.out.println("로그인 실패 - ID, Password를 확인해주세요");
				}
				else if( inputMsg.split(" ")[0].equals(LOGOUT))
				{
					if( logout() )
						System.out.println("로그아웃 성공");
					else
						System.out.println("이미 로그아웃 되어있습니다");
				}
				else if( inputMsg.split(" ")[0].equals(ROLLBACK))
				{
					if( rollbackLectures() )
						System.out.println("되돌리기 성공");;
				}
				else if( inputMsg.split(" ")[0].equals(SEARCH))
					searchLectures();
				else if( inputMsg.split(" ")[0].equals(REGIST))
				{
					if( registWishLecture() )
						System.out.println("수강희망 등록완료");;
				}
				else if( inputMsg.split(" ")[0].equals(CANCEL))
				{
					if( cancelWishLecture() )
						System.out.println("수강희망 취소완료");;
				}
				else if( inputMsg.split(" ")[0].equals(ENROLL))
				{
					if( enrollLecture() )
						System.out.println("수강 신청 완료");;
				}
				else if( inputMsg.split(" ")[0].equals(WITHDRAW))
				{
					if( withdrawLecture() )
						System.out.println("수강 철회 완료");;
				}
				else if( inputMsg.split(" ")[0].equals(WISHLIST))
					printWishlist();
				else if( inputMsg.split(" ")[0].equals(TIMETABLE))
					printTimetable();
				else if( inputMsg.split(" ")[0].equals(OPEN))
				{
					if( openLecture() )
						System.out.println("강의 개설 완료");;
				}
				else if( inputMsg.split(" ")[0].equals(CLOSE))
				{
					if( closeLecture() )
						System.out.println("강의 폐지 완료");
				}
				else
					System.out.println("해석 불가능한 명령입니다");
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
		}
	}
	
	private static boolean login()
	{
		String id, pwd, sql;
		ArrayList<LoginDTO> result;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		try 
		{
			if( user != null || isAdmin == true )
			{
				System.out.println("이미 로그인 되어있습니다, 로그아웃 - /logout");
				return false;
			}
			System.out.print("아이디 : ");
			id = sc.nextLine();
			
			System.out.print("패스워드 : ");
			pwd = sc.nextLine();
			

			if( id != null && id.equals(ADMIN_ID) && pwd != null && pwd.equals(ADMIN_PWD) )
			{
				user = new LoginDTO(ADMIN_ID, null, "관리자", null, null, null, null, new Timestamp(new Date().getTime()));
				isAdmin = true;
			}
			else
			{
				types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR));
				items = new ArrayList<Object>(Arrays.asList(id, pwd));
				
				sql = 	"SELECT " +
						"	student_id, 'password' AS password, name, sex, major_id, tutor_id, year, current_timestamp() AS lastlogin " +
						"FROM " +
						"	student " +
						"WHERE " +
						"	student_id = ? and password = ?";
				
				result = dbmanager.<LoginDTO>selectDynamicQuery(LoginDTO.class, sql, types, items);
				
				if( result != null && result.size() > 0)
				{
					user = result.get(0);
					isAdmin = false;
				}
			}
			
			return user != null;
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
	
	private static boolean logout()
	{
		if( user == null )
			return false;
		else
			user = null;
		
		if( isAdmin )
		{
			System.out.println("관리자 로그아웃");
			isAdmin = false;
		}
		
		return true;
	}
	
	private static boolean rollbackLectures()
	{
		String sql;
		boolean result;
		ArrayList<Integer> types = new ArrayList<Integer>(Arrays.asList(Types.TIMESTAMP, Types.TIMESTAMP));
		ArrayList<Object> items = new ArrayList<Object>(Arrays.asList(user.getLastlogin(), user.getLastlogin()));
		
		if( isAdmin )
		{
			System.out.println("관리자는 사용 불가능한 기능입니다.");
			return false;
		}
		
		try 
		{
			sql = 	"UPDATE enrollment " +
					"	set wished = lastwished, enrolled = lastenrolled, lastupdated = ? " +
					"WHERE " +
					"	lastupdated > ?";
			
			result = dbmanager.updateDynamicQuery(sql, types, items);

			return result;
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
	
	private static String convertCharToDay(char c)
	{
		String result = "? ";

		switch( c )
		{
		case '0':
			result = "일 ";
			break;
		case '1':
			result = "월 ";
			break;
		case '2':
			result = "화 ";
			break;
		case '3':
			result = "수 ";
			break;
		case '4':
			result = "목 ";
			break;
		case '5':
			result = "금 ";
			break;
		case '6':
			result = "토 ";
			break;
		default:
			result = "? ";
		}
		
		return result;
	}
	
	private static String convertLectureDateTime(LectureDTO lecture)
	{
		String result = "온라인강의/현장실습강의";
		
		if( lecture.getBegin() != null && lecture.getEnd() != null )
		{
			result = convertCharToDay(lecture.getBegin().charAt(9));
			result += lecture.getBegin().substring(0, 5);
			result += "\t";
			result += convertCharToDay(lecture.getEnd().charAt(9));
			result += lecture.getEnd().substring(0, 5);
		}
		
		return result;
	}
	
	private static boolean searchLectures()
	{
		String msg[], tmp[];
		String sql, mode = null;
		
		ArrayList<LectureDTO> result;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		if( user == null || inputMsg == null )
		{
			System.out.println("로그인이 필요한 기능입니다.");
			return false;
		}
		
		msg = inputMsg.split(" ");
		if ( msg.length > 1 && msg[1].charAt(0) == '-' )
		{
			tmp = msg[1].split("-");
			if( tmp.length > 1 )
			{
				mode = tmp[1];
			
				if( !(mode.equals("name") || mode.equals("clsn") || mode.equals("crsn") || mode.equals("prof")) )
				{
					mode = "name";
					System.out.println("/search -[mode=name(default, 강의명)|clsn(강의번호)|crsn(학수번호)|prof(교강사명)], [-mode] 명령어 해독 불가, 강의명으로 기본검색");
				}
			}
				
		}
		
		try 
		{
			if( mode != null )
			{
				System.out.print("키워드 : ");
				inputMsg = sc.nextLine();
				
				types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR));
				items = new ArrayList<Object>(Arrays.asList(inputMsg));
			}
			else
			{
				System.out.println("지정과목 출력");
				
				types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR));
				items = new ArrayList<Object>(Arrays.asList(user.getMajor_id(), user.getYear()));
			}

			sql = 	"SELECT " +
					"	class.*, lecturer.name AS lecturer_name, major.name AS major_name, building.name AS building_name, time.begin, time.end " +
					"FROM " +
					"	class, lecturer, major, room, building, time " +
					"WHERE " +
					"	class.opened = Year(CURDATE()) ";
			
			if( mode == null )
			{
				if( isAdmin == false)
					sql += "AND class.major_id = ? AND class.year = ? ";
			}
			else if( mode.equals("clsn") )
				sql += "AND class_no = ? ";
			else if( mode.equals("crsn") )
				sql += "AND course_id = ? ";
			else if( mode.equals("prof") )
				sql += "AND lecturer.name like concat(?, '%') ";
			else if( mode.equals("name"))
				sql += "AND class.name like concat('%', ?, '%') ";
			
			sql += 	"AND class.lecturer_id = lecturer.lecturer_id " +
					"AND class.major_id = major.major_id " +
					"AND class.room_id = room.room_id " +
					"AND room.building_id = building.building_id " +
					"AND class.class_id = time.class_id";
			
			System.out.println("재수강\t대상학년\t관장학부\t강의번호\t학수번호\t교과목명\t교강사명\t수강학점\t건물명\t강의실\t설강연도\t수강/정원\t강의시작시간\t강의종료시간");
			result = dbmanager.<LectureDTO>selectDynamicQuery(LectureDTO.class, sql, types, items);
			
			if( result != null )
			{
				String prevClassNo = "";
				for( LectureDTO lecture : result )
				{
					if( prevClassNo != null && prevClassNo.equals(lecture.getClass_no()) )
					{
						System.out.println
						(
							"\t\t\t\t\t\t\t\t\t\t\t\t" + 
							convertLectureDateTime(lecture)
						);
					}
					else
					{
						System.out.println
						(
							getIsRetaken(lecture.getCourse_id()) + "\t" + 
							lecture.getYear() + "\t" +
							(lecture.getMajor_name().length() > 6 ? lecture.getMajor_name().substring(0, 6) : lecture.getMajor_name()) + "\t" +
							lecture.getClass_no() + "\t" + 
							lecture.getCourse_id() + "\t" +
							(lecture.getName().length() > 6 ? lecture.getName().substring(0, 6) : lecture.getName()) + "\t" +
							(lecture.getLecturer_name().length() > 6 ? lecture.getLecturer_name().substring(0, 6) : lecture.getLecturer_name()) + "\t" +
							lecture.getCredit() + "\t" +
							lecture.getBuilding_name() + "\t" +	lecture.getRoom_id() + "호\t" +
							lecture.getOpened() + "\t" +
							lecture.getPerson_max() + "\t" +
							convertLectureDateTime(lecture)
						);
					}
					
					if( lecture != null )
						prevClassNo = lecture.getClass_no();
				}
			}
			
			return result != null;
		}
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
	
	private static String getIsRetaken(String courseID) 
	{
		String result = "N";
		String sql;
		
		ArrayList<EnrollmentDTO> prev;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR));
		items = new ArrayList<Object>(Arrays.asList(user.getStudent_id(), courseID));

		sql = 	"SELECT * FROM enrollment WHERE student_id = ? AND course_id = ? AND opened < YEAR(CURDATE()) AND achievement IS NOT NULL";
		prev = dbmanager.<EnrollmentDTO>selectDynamicQuery(EnrollmentDTO.class, sql, types, items);
		
		if( prev.size() > 0 )
		{
			for( EnrollmentDTO enrolled : prev )
			{
				if( enrolled.getAchievement().compareTo("B+") <= 0 || enrolled.getAchievement().equals("P") )
				{
					System.out.println("B0 이상 이전 성적이 존재합니다 - " + enrolled.getAchievement());
					return result = "X";
				}
			}
			
			result = "Y";
		}

		return result;
	}
	
	private static boolean isDuplicated(LectureDTO lecture) 
	{
		String sql;

		ArrayList<EnrollmentDTO> enrollments;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR, Types.VARCHAR, Types.VARCHAR, Types.VARCHAR, Types.VARCHAR));
		items = new ArrayList<Object>
			(
				Arrays.asList
				(
					user.getStudent_id(), 
					lecture.getClass_no(), 
					lecture.getBegin(), 
					lecture.getBegin(), 
					lecture.getEnd(), 
					lecture.getEnd()
				)
			);

		sql = 	"SELECT enrollment.* FROM time, enrollment, class " + 
				"WHERE enrollment.student_id = ? " +
				"AND enrollment.enrolled = 1 " +
				"AND enrollment.opened = YEAR(CURDATE()) AND enrollment.achievement IS NULL " +
				"AND enrollment.class_no <> ? " 	+ 
				"AND class.class_no = enrollment.class_no " +  
				"AND time.class_id = class.class_id " +  
				"AND ((time.begin <= ? && ? <= time.end) OR (time.begin <= ? && ? <= time.end)) " +
				"ORDER BY enrollment.class_no " +
				"LIMIT 1";
		
		enrollments = dbmanager.<EnrollmentDTO>selectDynamicQuery(EnrollmentDTO.class, sql, types, items);

		return enrollments.size() > 0;
	}
	
	private static boolean isFull(LectureDTO lecture) 
	{
		String sql;

		ArrayList<BigintDTO> cnts;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR));
		items = new ArrayList<Object>(Arrays.asList(lecture.getClass_no()));

		sql = 	"SELECT count(*) AS number FROM enrollment WHERE enrolled = 1 " +
				"AND opened = YEAR(CURDATE()) AND achievement IS NULL AND class_no = ?";
		
		cnts = dbmanager.<BigintDTO>selectDynamicQuery(BigintDTO.class, sql, types, items);

		return cnts.size() > 0 && cnts.get(0).getNumber().intValue() >= lecture.getPerson_max();
	}
	
	private static boolean confirmEnrollmentPossible(String classNO) 
	{
		String sql;
		
		ArrayList<LectureDTO> prev;
		ArrayList<Integer> types;
		ArrayList<Object> items;

		types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR));
		items = new ArrayList<Object>(Arrays.asList(classNO));

		sql = 	"SELECT class.*, '' AS lecturer_name, '' AS major_name, '' AS building_name, " +
				"time.begin AS begin, time.end AS end " +
				"FROM class, time WHERE class.class_no = ? AND class.opened = YEAR(CURDATE()) " + 
				"AND class.class_id = time.class_id";
		prev = dbmanager.<LectureDTO>selectDynamicQuery(LectureDTO.class, sql, types, items);
		
		if( prev.size() > 0 )
		{
			if( isDuplicated(prev.get(0)) )
			{
				System.out.println("수강신청이 된 과목들 중 시간이 겹치는 과목이 존재합니다.");
				return false;
			}
			if( isFull(prev.get(0)) )
			{
				System.out.println("수강인원이 꽉 차 신청이 불가능합니다.");
				return false;
			}
			if( getIsRetaken(prev.get(0).getCourse_id()).equals("X") == true )
			{
				System.out.println("B학점 이상의 성적이 있는 경우 재수강이 불가능하니다.");
				return false;
			}
			if( user.getMajor_id().equals(prev.get(0).getMajor_id()) == false)
			{
				System.out.println("해당 전공 학생만 수강이 가능한 강의입니다.");
				return false;
			}
			if( user.getYear().compareTo(prev.get(0).getYear()) < 0 )
			{
				System.out.println(prev.get(0).getYear() + "학년 이상만 수강이 가능한 과목입니다.");
				return false;
			}
		}
		else
		{
			System.out.println("존재하지 않는 강좌이거나 존재하는 강좌일 경우 운영자에게 도움을 요청하세요.");
			return false;
		}

		return true;
	}

	@SuppressWarnings("unused")
	private static boolean registWishLecture()
	{
		String msg[];
		String sql, classNo = "";
		
		ArrayList<EnrollmentDTO> selected;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		boolean result = false;
		
		if( isAdmin )
		{
			System.out.println("관리자는 사용 불가능한 기능입니다.");
			return false;
		}
		
		if( user == null || inputMsg == null )
		{
			System.out.println("로그인이 필요한 기능입니다.");
			return false;
		}
		
		msg = inputMsg.split(" ");
		if ( msg.length > 1 )
			classNo = msg[1];
		else
		{
			System.out.println("강의번호를 입력하세요");
			return false;
		}
		try 
		{
			types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR));
			items = new ArrayList<Object>(Arrays.asList(classNo, user.getStudent_id()));
			sql = 	"SELECT * FROM enrollment WHERE class_no = ? AND student_id = ? AND opened = Year(CURDATE()) AND achievement IS NULL";
			
			selected = dbmanager.<EnrollmentDTO>selectDynamicQuery(EnrollmentDTO.class, sql, types, items);
			
			if( selected.size() > 0 )
			{
				if( selected.get(0).getWished() == 1 )
				{
					System.out.println("이미 희망수강신청 된 강의입니다.");
					result = false;
				}
				else
				{
					types = new ArrayList<Integer>(Arrays.asList(Types.TIMESTAMP, Types.TIMESTAMP, Types.VARCHAR, Types.VARCHAR));
					items = new ArrayList<Object>(Arrays.asList(user.getLastlogin(), user.getLastlogin(), classNo, user.getStudent_id()));
					
					sql = 	"UPDATE " +
							"	enrollment " + 
							"SET " +
							"	lastwished = (CASE WHEN lastupdated < ? THEN wished ELSE lastwished END), " +
							"	lastenrolled = (CASE WHEN lastupdated < ? THEN enrolled ELSE lastenrolled END), " +
							"	wished = 1, " +
							"	lastupdated = current_timestamp " +
							"WHERE " +
							"	class_no = ? AND student_id = ? AND opened = Year(CURDATE()) AND achievement IS NULL";
					
					result = dbmanager.<EnrollmentDTO>updateDynamicQuery(sql, types, items);
				}
			}
			else
			{
				types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR, Types.VARCHAR));
				items = new ArrayList<Object>(Arrays.asList(classNo, classNo, user.getStudent_id()));
				sql = 	"INSERT INTO enrollment " + 
						"VALUE(?, (" + 
						"SELECT " + 
						"	course_id " + 
						"FROM " +
						"	class " + 
						"WHERE " + 
						"	class_no = ? AND opened = Year(CURDATE()) " + 
						"ORDER BY " +
						"	class_no " + 
						"LIMIT 1 " +
						"), ?, 1, 0, 0, 0, current_timestamp, Year(CURDATE()), NULL)";
				
				result = dbmanager.<EnrollmentDTO>updateDynamicQuery(sql, types, items);
			}
			
			return result;
		}
		catch (Exception e) 
		{
			System.out.println("존재하지 않는 강좌이거나 존재하는 강좌일 경우 운영자에게 도움을 요청하세요.");
			e.printStackTrace();
			return false;
		}
	}
	
	@SuppressWarnings("unused")
	private static boolean cancelWishLecture()
	{
		boolean result;
		String msg[];
		String sql, classNo = "";
		
		ArrayList<EnrollmentDTO> selected;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		if( isAdmin )
		{
			System.out.println("관리자는 사용 불가능한 기능입니다.");
			return false;
		}
		
		if( user == null || inputMsg == null )
		{
			System.out.println("로그인이 필요한 기능입니다.");
			return false;
		}
		
		msg = inputMsg.split(" ");
		if ( msg.length > 1 )
			classNo = msg[1];
		else
		{
			System.out.println("강의번호를 입력하세요");
			return false;
		}
		
		types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR));
		items = new ArrayList<Object>(Arrays.asList(classNo, user.getStudent_id()));
		try 
		{
			sql = 	"SELECT * FROM enrollment WHERE class_no = ? AND student_id = ? AND opened = Year(CURDATE()) AND achievement IS NULL";
			
			selected = dbmanager.<EnrollmentDTO>selectDynamicQuery(EnrollmentDTO.class, sql, types, items);
			
			if( selected.size() > 0 )
			{
				if( selected.get(0).getWished() == 0 )
				{
					System.out.println("등록되지 않은 희망수강신청 강의입니다.");
					result = false;
				}
				else
				{
					types = new ArrayList<Integer>(Arrays.asList(Types.TIMESTAMP, Types.TIMESTAMP, Types.VARCHAR, Types.VARCHAR));
					items = new ArrayList<Object>(Arrays.asList(user.getLastlogin(), user.getLastlogin(), classNo, user.getStudent_id()));
					
					sql = 	"UPDATE " +
							"	enrollment " + 
							"SET " +
							"	lastwished = (CASE WHEN lastupdated < ? THEN wished ELSE lastwished END), " +
							"	lastenrolled = (CASE WHEN lastupdated < ? THEN enrolled ELSE lastenrolled END), " +
							"	wished = 0, " +
							"	lastupdated = current_timestamp " +
							"WHERE " +
							"	class_no = ? AND student_id = ? AND opened = Year(CURDATE()) AND achievement IS NULL";
					
					result = dbmanager.<EnrollmentDTO>updateDynamicQuery(sql, types, items);
				}
			}
			else
			{
				System.out.println("등록되지 않은 희망수강신청 강의입니다.");
				result = false;
			}
			
			return result;
		}
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
	
	@SuppressWarnings("unused")
	private static boolean enrollLecture()
	{
		boolean result;
		String msg[];
		String sql, classNo = "";
		
		ArrayList<EnrollmentDTO> selected;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		if( isAdmin )
		{
			System.out.println("관리자는 사용 불가능한 기능입니다.");
			return false;
		}
		
		if( user == null || inputMsg == null )
		{
			System.out.println("로그인이 필요한 기능입니다.");
			return false;
		}
		
		msg = inputMsg.split(" ");
		if ( msg.length > 1 )
			classNo = msg[1];
		else
		{
			System.out.println("강의번호를 입력하세요");
			return false;
		}
		try 
		{
			if(confirmEnrollmentPossible(classNo) == false )
			{
				System.out.println("수강신청에 실패하였습니다.");
				return false;
			}
			
			types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR));
			items = new ArrayList<Object>(Arrays.asList(classNo, user.getStudent_id()));
			sql = 	"SELECT * FROM enrollment WHERE class_no = ? AND student_id = ? AND opened = YEAR(CURDATE())";
			
			selected = dbmanager.<EnrollmentDTO>selectDynamicQuery(EnrollmentDTO.class, sql, types, items);
			
			if( selected.size() > 0 )
			{
				if( selected.get(0).getEnrolled() == 1 )
				{
					System.out.println("이미 신청된 강의입니다.");
					result = false;
				}
				else
				{
					types = new ArrayList<Integer>(Arrays.asList(Types.TIMESTAMP, Types.TIMESTAMP, Types.VARCHAR, Types.VARCHAR));
					items = new ArrayList<Object>(Arrays.asList(user.getLastlogin(), user.getLastlogin(), classNo, user.getStudent_id()));
					
					sql = 	"UPDATE " +
							"	enrollment " + 
							"SET " +
							"	lastwished = (CASE WHEN lastupdated < ? THEN wished ELSE lastwished END), " +
							"	lastenrolled = (CASE WHEN lastupdated < ? THEN enrolled ELSE lastenrolled END), " +
							"	enrolled = 1, " +
							"	lastupdated = current_timestamp " +
							"WHERE " +
							"	class_no = ? AND student_id = ? AND opened = Year(CURDATE()) AND achievement IS NULL";
					
					result = dbmanager.<EnrollmentDTO>updateDynamicQuery(sql, types, items);
					
					return result;
				}
			}
			else
			{
				types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR, Types.VARCHAR));
				items = new ArrayList<Object>(Arrays.asList(classNo, classNo, user.getStudent_id()));

				sql = 	"INSERT INTO enrollment " + 
						"VALUE(?, (" + 
						"SELECT " + 
						"	course_id " + 
						"FROM " +
						"	class " + 
						"WHERE " + 
						"	class_no = ? AND opened = Year(CURDATE()) " + 
						"ORDER BY " +
						"	class_no " + 
						"LIMIT 1 " +
						"), ?, 0, 1, 0, 0, current_timestamp, Year(CURDATE()), NULL)";
				
				result = dbmanager.<EnrollmentDTO>updateDynamicQuery(sql, types, items);
			}
			
			return result;
		}
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
	
	@SuppressWarnings("unused")
	private static boolean withdrawLecture()
	{
		boolean result;
		String msg[];
		String sql, classNo = "";
		
		ArrayList<EnrollmentDTO> selected;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		if( isAdmin )
		{
			System.out.println("관리자는 사용 불가능한 기능입니다.");
			return false;
		}
		
		if( user == null || inputMsg == null )
		{
			System.out.println("로그인이 필요한 기능입니다.");
			return false;
		}
		
		msg = inputMsg.split(" ");
		if ( msg.length > 1 )
			classNo = msg[1];
		else
		{
			System.out.println("강의번호를 입력하세요");
			return false;
		}
		
		types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR));
		items = new ArrayList<Object>(Arrays.asList(classNo, user.getStudent_id()));
		try 
		{
			sql = 	"SELECT * FROM enrollment WHERE class_no = ? AND student_id = ? AND opened = Year(CURDATE()) AND achievement IS NULL";
			
			selected = dbmanager.<EnrollmentDTO>selectDynamicQuery(EnrollmentDTO.class, sql, types, items);
			
			if( selected.size() > 0 )
			{
				if( selected.get(0).getEnrolled() == 0 )
				{
					System.out.println("신청되지 않은 강의입니다.");
					result = false;
				}
				else
				{
					types = new ArrayList<Integer>(Arrays.asList(Types.TIMESTAMP, Types.TIMESTAMP, Types.VARCHAR, Types.VARCHAR));
					items = new ArrayList<Object>(Arrays.asList(user.getLastlogin(), user.getLastlogin(), classNo, user.getStudent_id()));
					
					sql = 	"UPDATE " +
							"	enrollment " + 
							"SET " +
							"	lastwished = (CASE WHEN lastupdated < ? THEN wished ELSE lastwished END), " +
							"	lastenrolled = (CASE WHEN lastupdated < ? THEN enrolled ELSE lastenrolled END), " +
							"	enrolled = 0, " +
							"	lastupdated = current_timestamp " +
							"WHERE " +
							"	class_no = ? AND student_id = ? AND opened = Year(CURDATE()) AND achievement IS NULL";
					
					result = dbmanager.<EnrollmentDTO>updateDynamicQuery(sql, types, items);
				}
			}
			else
				result = false;
			
			return result;
		}
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
	
	private static boolean printWishlist()
	{
		String sql;
		
		ArrayList<LectureDTO> result;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		if( isAdmin )
		{
			System.out.println("관리자는 사용 불가능한 기능입니다.");
			return false;
		}
		
		if( user == null || inputMsg == null )
		{
			System.out.println("로그인이 필요한 기능입니다.");
			return false;
		}

		types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR));
		items = new ArrayList<Object>(Arrays.asList(user.getStudent_id()));
		try 
		{
			sql = 	"SELECT " +
					"	class.*, lecturer.name AS lecturer_name, major.name AS major_name, building.name AS building_name, time.begin, time.end " +
					"FROM " +
					"	class, lecturer, major, room, building, time, enrollment " +
					"WHERE " +
					"	class.opened = Year(CURDATE()) " +
					"	AND class.lecturer_id = lecturer.lecturer_id " +
					"	AND class.major_id = major.major_id " +
					"	AND class.room_id = room.room_id " +
					"	AND room.building_id = building.building_id " +
				    "	AND enrollment.student_id = ? " +
					"   AND enrollment.opened = Year(CURDATE()) AND enrollment.achievement IS NULL " +
				    "	AND (enrollment.enrolled = 0 OR enrollment.wished = 1)" +
				    "	AND enrollment.class_no = class.class_no " +
					"	AND class.class_id = time.class_id " +
					"ORDER BY " +
					"	begin ASC";
			
			System.out.println("수강희망  목록 중 아직 신청이 이뤄지지 않은 목록입니다.");
			System.out.println("요일\t과목명\t건물명\t강의실\t교강사\t시작시간\t종료시간");
			result = dbmanager.<LectureDTO>selectDynamicQuery(LectureDTO.class, sql, types, items);
			
			if( result != null )
			{
				String prevDay = "";
				for( LectureDTO lecture : result )
				{
					if( prevDay == null || !(prevDay.equals(convertCharToDay(lecture.getBegin().charAt(9)))) )
						System.out.print(convertCharToDay(lecture.getBegin().charAt(9)));
						
					System.out.println
					(
						"\t" +
						(lecture.getName().length() > 6 ? lecture.getName().substring(0, 6) : lecture.getName()) + "\t" +
						lecture.getBuilding_name() + "\t" +	lecture.getRoom_id() + "호\t" +
						(lecture.getLecturer_name().length() > 6 ? lecture.getLecturer_name().substring(0, 6) : lecture.getLecturer_name()) + "\t" +
						lecture.getBegin().split("T")[1].substring(0, 5) + "\t" +
						lecture.getEnd().split("T")[1].substring(0, 5)
					);
					
					if( lecture != null )
						prevDay = convertCharToDay(lecture.getBegin().charAt(9));
				}
			}
			
			return result != null;
		}
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
	
	private static boolean printTimetable()
	{
		String sql;
		
		ArrayList<LectureDTO> result;
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		if( isAdmin )
		{
			System.out.println("관리자는 사용 불가능한 기능입니다.");
			return false;
		}
		
		if( user == null || inputMsg == null )
		{
			System.out.println("로그인이 필요한 기능입니다.");
			return false;
		}

		types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR));
		items = new ArrayList<Object>(Arrays.asList(user.getStudent_id()));
		try 
		{
			sql = 	"SELECT " +
					"	class.*, lecturer.name AS lecturer_name, major.name AS major_name, building.name AS building_name, time.begin, time.end " +
					"FROM " +
					"	class, lecturer, major, room, building, time, enrollment " +
					"WHERE " +
					"	class.opened = Year(CURDATE()) " +
					"	AND class.lecturer_id = lecturer.lecturer_id " +
					"	AND class.major_id = major.major_id " +
					"	AND class.room_id = room.room_id " +
					"	AND room.building_id = building.building_id " +
				    "	AND enrollment.student_id = ? " +
					"   AND enrollment.opened = Year(CURDATE()) AND enrollment.achievement IS NULL " +
				    "	AND enrollment.enrolled = 1 " +
				    "	AND enrollment.class_no = class.class_no " +
					"	AND class.class_id = time.class_id " +
					"ORDER BY " +
					"	begin ASC";

			System.out.println("신청된 강의 시간표");
			System.out.println("요일\t과목명\t건물명\t강의실\t교강사\t시작시간\t종료시간");
			result = dbmanager.<LectureDTO>selectDynamicQuery(LectureDTO.class, sql, types, items);
			
			if( result != null )
			{
				String prevDay = "";
				for( LectureDTO lecture : result )
				{
					if( prevDay == null || !(prevDay.equals(convertCharToDay(lecture.getBegin().charAt(9)))) )
						System.out.print(convertCharToDay(lecture.getBegin().charAt(9)));
						
					System.out.println
					(
						"\t" +
						(lecture.getName().length() > 6 ? lecture.getName().substring(0, 6) : lecture.getName()) + "\t" +
						lecture.getBuilding_name() + "\t" +	lecture.getRoom_id() + "호\t" +
						(lecture.getLecturer_name().length() > 6 ? lecture.getLecturer_name().substring(0, 6) : lecture.getLecturer_name()) + "\t" +
						lecture.getBegin().split("T")[1].substring(0, 5) + "\t" +
						lecture.getEnd().split("T")[1].substring(0, 5)
					);
					
					if( lecture != null )
						prevDay = convertCharToDay(lecture.getBegin().charAt(9));
				}
			}
			
			return result != null;
		}
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
	
	@SuppressWarnings("unused")
	private static boolean openLecture()
	{
		String sql;
		
		ArrayList<String> classinfo = new ArrayList<String>();
		ArrayList<String> timeinfo = new ArrayList<String>();
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		int day, hour, min;
		int weeksOfClasses = 0;
		
		boolean result = false;
		
		if( user == null || inputMsg == null )
		{
			System.out.println("로그인이 필요한 기능입니다.");
			return false;
		}

		System.out.print("강의번호(class_no) 입력 : ");
		classinfo.add(sc.nextLine());
		System.out.print("과정ID(course_id) 입력 : ");
		classinfo.add(sc.nextLine());
		System.out.print("학부번호(major_id) 입력 : ");
		classinfo.add(sc.nextLine());
		System.out.print("대상학년(year) 입력 : ");
		classinfo.add(sc.nextLine());
		System.out.print("교강사ID(lecturer_id) 입력 : ");
		classinfo.add(sc.nextLine());
		System.out.print("최대수강가능인원(person_max, 숫자) 입력 : ");
		classinfo.add(sc.nextLine());
		System.out.print("개설년도(opened) 입력 : ");
		classinfo.add(sc.nextLine());
		System.out.print("강의실ID(room_id) 입력 : ");
		classinfo.add(sc.nextLine());
		
		try 
		{
			System.out.print("주별 강의 진행 회수 입력 : ");
			weeksOfClasses = Integer.parseInt(sc.nextLine());
			
			for( int i = 0 ; i < weeksOfClasses ; i++ )
			{
				System.out.print("시작시간(begin, D[0(일)~6(토)]/hh[00~24]/mm[00~59]) 입력 : ");
				timeinfo.add(sc.nextLine());
				System.out.print("종료시간(end, D[0(일)~6(토)]/hh[00~24]/mm[00~59]) 입력 : ");
				timeinfo.add(sc.nextLine());
				
				/*정상적으로 포맷에 맞게 입력 했는지 확인하는 코드*/
				day 	= Integer.parseInt(timeinfo.get(2 * i).split("/")[0]); 
				hour 	= Integer.parseInt(timeinfo.get(2 * i).split("/")[1]); 
				min		= Integer.parseInt(timeinfo.get(2 * i).split("/")[2]);
				
				if( day < 0 || 6 < day || hour < 0 || 24 <= hour || min < 0 || 60 <= min )
					throw new Exception("Input Error!!");

				day 	= Integer.parseInt(timeinfo.get(2 * i + 1).split("/")[0]); 
				hour	= Integer.parseInt(timeinfo.get(2 * i + 1).split("/")[1]); 
				min 	= Integer.parseInt(timeinfo.get(2 * i + 1).split("/")[2]);
				
				if( day < 0 || 6 < day || hour < 0 || hour >= 24 || min < 0 || 60 <= min )
					throw new Exception("Input Error!!");
			}

			ArrayList<DoubleDTO> cnts;
			
			types = new ArrayList<Integer>();
			items = new ArrayList<Object>();

			sql = "(SELECT (MAX(class_id) + 1) AS number from class where LENGTH(class_id) <= 4)";
			
			cnts = dbmanager.<DoubleDTO>selectDynamicQuery(DoubleDTO.class, sql, types, items);

			String nextClassID = Integer.toString(cnts.size() > 0 ? (int)(cnts.get(0).getNumber()) : 0);
			
			types = new ArrayList<Integer>
				(
					Arrays.asList
					(
						Types.VARCHAR,
						Types.VARCHAR,
						Types.VARCHAR,
						Types.VARCHAR,
						Types.VARCHAR,
						Types.VARCHAR,
						Types.VARCHAR,
						Types.VARCHAR,
						Types.INTEGER,
						Types.VARCHAR, 
						Types.VARCHAR
					)
				);
			items = new ArrayList<Object>
				(
					Arrays.asList
					(
						nextClassID,
						classinfo.get(0),
						classinfo.get(1),
						classinfo.get(1),
						classinfo.get(2),
						classinfo.get(3),
						classinfo.get(1),
						classinfo.get(4),
						Integer.parseInt(classinfo.get(5)),
						classinfo.get(6),
						classinfo.get(7)
					)
				);
			
			sql =	 "INSERT INTO class VALUE(" +
					 "?, ?, ?, " + 
					 "(SELECT name FROM course WHERE course_id=?), ?, ?, " + 
					 "(SELECT credit FROM course WHERE course_id=?), ?, ?, ?, ?)";
			
			result = dbmanager.<EnrollmentDTO>updateDynamicQuery(sql, types, items);
			
			if( result )
			{
				types = new ArrayList<Integer>
				(
					Arrays.asList
					(
						Types.VARCHAR,
						Types.VARCHAR,
						Types.TIMESTAMP,
						Types.TIMESTAMP
					)
				);
				
				for( int i = 0 ; i < weeksOfClasses ; i++ )
				{
					@SuppressWarnings("deprecation")
					Timestamp begin = new Timestamp
						(
							0, 
							0, 
							Integer.parseInt(timeinfo.get(2 * i).split("/")[0]), 
							Integer.parseInt(timeinfo.get(2 * i).split("/")[1]), 
							Integer.parseInt(timeinfo.get(2 * i).split("/")[2]), 
							0, 
							0
						);
					@SuppressWarnings("deprecation")
					Timestamp end = new Timestamp
						(
							0, 
							0, 
							Integer.parseInt(timeinfo.get(2 * i + 1).split("/")[0]), 
							Integer.parseInt(timeinfo.get(2 * i + 1).split("/")[1]), 
							Integer.parseInt(timeinfo.get(2 * i + 1).split("/")[2]), 
							0, 
							0
						);

					items = new ArrayList<Object>
						(
							Arrays.asList
							( 	
								nextClassID,
								Integer.toString(i + 1),
								begin,
								end
							)
						);
					
					sql = 	"INSERT INTO time VALUE((SELECT TMP.MAXTIME FROM (SELECT (MAX(CAST(time_id AS UNSIGNED)) + 1) AS MAXTIME from time) TMP), " +
							"?, ?, DATE_FORMAT(?, '%Y-%m-%dT%H:%i:%s.000Z'), DATE_FORMAT(?, '%Y-%m-%dT%H:%i:%s.000Z'))";
					
					result = dbmanager.<EnrollmentDTO>updateDynamicQuery(sql, types, items);
				}
			}
			
			return result;
		}
		catch (NumberFormatException nfe)
		{
			System.out.println("학점과 최대수강 가능인원은 숫자로만 입력되야 합니다");
			nfe.printStackTrace();
	        return false;
	    }
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
	
	@SuppressWarnings("unused")
	private static boolean closeLecture()
	{
		String sql, classinfo[] = new String[2];
		ArrayList<Integer> types;
		ArrayList<Object> items;
		
		boolean result = false;
		
		if( user == null || inputMsg == null )
		{
			System.out.println("로그인이 필요한 기능입니다.");
			return false;
		}
		
		System.out.print("강의ID(class_id) 입력 : ");
		classinfo[0] = sc.nextLine();
		
		System.out.print("강의번호(class_no) 입력 : ");
		classinfo[1] = sc.nextLine();
		
		try 
		{
			types = new ArrayList<Integer>(Arrays.asList(Types.VARCHAR, Types.VARCHAR));
			items = new ArrayList<Object>(Arrays.asList(classinfo[0], classinfo[1]));
			
			sql = "DELETE FROM class WHERE class_id = ? AND class_no = ?";
			
			result = dbmanager.<EnrollmentDTO>updateDynamicQuery(sql, types, items);
			
			return result;
		}
		catch (Exception e) 
		{
			e.printStackTrace();
			return false;
		}
	}
}