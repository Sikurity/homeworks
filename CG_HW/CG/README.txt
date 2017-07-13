This is an Lee YeongSik's Computer Graphic Practice Assignment Report
==========================================================================================================================================

추가 및 수정 코드 내용 및 줄 번호
------------------------------------------------------------------------------------------------------------------------------------------
#LINE 48 ~ 59
int cntCow;					// 사용자가 복사시킨 소 오브젝트 개수 카운트

matrix4 dupCow[6];			// 사용자가 Ctrl을 눌러 복사를 누르면 dupCow[cntCow]에 cow2wld가 복사되어 저장되는 변수
vector3 prevCow;			// 시뮬레이션 되는 동안, 소가 바라보는 방향을 계산하기 위해 이전 소의 위치를 저장하기 위한 변수

double bspCoefX[3][4];		// bspCoef[0] : 1번째 B-Spline Curve, bspCoef[1] : 2번째 B-Spline Curve, bspCoef[2] : 3번째 B-Spline Curve
double bspCoefZ[3][4];		// bspCoef[n][0] : 3차항 계수, bspCoef[n][1] : 2차항 계수, bspCoef[n][2] : 1차항 계수, bspCoef[n][3] : 0차항 계수
double curTime;				// 시뮬레이션이 이뤄지는 동안 0.0f ~ 3.0f 으로 순환하는 시간 변수

bool bReady;				// 위치가 모두 선정되면 선정된 6개의 점의 위치로 B-Spline 함수의 계수 3쌍을 구해 저장한 후 false -> true로 바뀜
bool bClicked;				// 소를 선택했는지 여부를 저장하는 변수
bool isCowReady[6];			// 사용자가 Ctrl을 눌러 복사를 누르면 isCowReady[cntCow] : false -> true로 바뀌고, true일 경우 출력

#LINE 307 ~ 360
if( bReady )						// 점 6개로, B-Spling 곡선 3개가 완성되면 실행되는 분기문
{
	curTime = glfwGetTime();			// 현재 시간을 가져온다

	// 0 ~ 3초 동안만 시뮬레이션 하는 것이므로, 3초 초과시 모든 것을 초기화
	if (curTime >= 3.0f)
	{
		cntCow = 0;
		curTime = 0.0f;
		bReady = false;

		memset(isCowReady, 0, sizeof(isCowReady));
		memset(dupCow, 0, sizeof(dupCow));
		memset(bspCoefX, 0, sizeof(bspCoefX));
		memset(bspCoefZ, 0, sizeof(bspCoefZ));

		prevCow.x = 0.0f;
		prevCow.y = 0.0f;
		prevCow.z = 0.0f;

		cow2wld.setTranslation(prevCow);
		cow2wld.setRotationY(-M_PI / 2);
	}
	else
	{
		t_time = curTime - (int)curTime;		// 현재 시간에서 소수점 부분만 사용, B-Spling 곡선이 0 <= t <= 1에 대해서 정의되기 때문

		// Bspline 함수의 계수들에 현재시간 t^n을 구해 곱해준다, 곱셈을 최소화 하기 위해 아래와 같이 곱해줌
		// 구한 좌표를 시뮬레이션 할 소 오브젝트의 위치 값으로 대입
		cowPos.x = bspCoefX[(int)curTime][3] + (bspCoefX[(int)curTime][2] + t_time * (bspCoefX[(int)curTime][1] + t_time * bspCoefX[(int)curTime][0])) * t_time;
		cowPos.y = 0.0f;
		cowPos.z = bspCoefZ[(int)curTime][3] + (bspCoefZ[(int)curTime][2] + t_time * (bspCoefZ[(int)curTime][1] + t_time * bspCoefZ[(int)curTime][0])) * t_time;

		// 시뮬레이션 중 소가 바라보는 방향을 구하기 위하여, 직전 위치에서 현재 위치로의 방향 벡터를 구함
		cowDir = cowPos - prevCow;
		
		// 위에서 구한 방향 벡터에서 arctan를 이용해 회전각을 구해 y축에 대해 회전 시킴, arctan에 음수 부호를 붙인 것은 보정용
		cow2wld.setRotationY(-atan2(cowDir.z, cowDir.x));
		cow2wld.setTranslation(cowPos);

		// 이전 위치를 저장해 놓는 변수를 현재 위치로 갱신
		prevCow = cowPos;
	}
}
// 아직 B-Spline을 만들 점의 개수가 부족하면 실행되는 분기문
else
{
	// ctrl을 누를 때 마다 늘어나는 복사된 소를 화면에 출력
	for( i = 0 ; i < 6 ; i++ )
	{
		if( isCowReady[i] )
			drawCow(dupCow[i], cursorOnCowBoundingBox);
	}
}

#LINE 509
static double p_x[6], p_z[6];

#LINE 521 ~ 569
if (!bClicked && cursorOnCowBoundingBox)	// 소를 최초로 클릭 한 경우, bClicked를 True로 바꿔줌
	bClicked = true;
else if (bClicked && bReady == false)	// Ctrl 키가 눌렸고 && 시뮬레이션 도중이 아닌경우
{
	isCowReady[cntCow] = true;		// 소가 복사되었는지 확인하는 변수
	dupCow[cntCow++] = cow2wld;;	// Ctrl키가 눌린 순간의 소 오브젝트를 복사

	// 복사된 소가 6마리라면, 점의 위치 6개가 모두 정해졌다는 뜻이므로, B-Spline 곡선을 생성한다
	if (cntCow == 6)
	{
		// 점 6개의 위치 저장
		p_x[0] = dupCow[0].getTranslation().x;
		p_z[0] = dupCow[0].getTranslation().z;

		p_x[1] = dupCow[1].getTranslation().x;
		p_z[1] = dupCow[1].getTranslation().z;

		p_x[2] = dupCow[2].getTranslation().x;
		p_z[2] = dupCow[2].getTranslation().z;

		p_x[3] = dupCow[3].getTranslation().x;
		p_z[3] = dupCow[3].getTranslation().z;

		p_x[4] = dupCow[4].getTranslation().x;
		p_z[4] = dupCow[4].getTranslation().z;

		p_x[5] = dupCow[5].getTranslation().x;
		p_z[5] = dupCow[5].getTranslation().z;

		// B-Spline 곡선 함수의 0, 1, 2, 3차 계수 생성
		for (i = 0; i < 3; i++)
		{
			bspCoefX[i][0] = (-p_x[i] + 3 * p_x[i + 1] - 3 * p_x[i + 2] + p_x[i + 3]) / 6.0f;
			bspCoefX[i][1] = (3 * p_x[i] - 6 * p_x[i + 1] + 3 * p_x[i + 2]) / 6.0f;
			bspCoefX[i][2] = (-3 * p_x[i] + 3 * p_x[i + 2]) / 6.0f;
			bspCoefX[i][3] = (p_x[i] + 4 * p_x[i + 1] + p_x[i + 2]) / 6.0f;

			bspCoefZ[i][0] = (-p_z[i] + 3 * p_z[i + 1] - 3 * p_z[i + 2] + p_z[i + 3]) / 6.0f;
			bspCoefZ[i][1] = (3 * p_z[i] - 6 * p_z[i + 1] + 3 * p_z[i + 2]) / 6.0f;
			bspCoefZ[i][2] = (-3 * p_z[i] + 3 * p_z[i + 2]) / 6.0f;
			bspCoefZ[i][3] = (p_z[i] + 4 * p_z[i + 1] + p_z[i + 2]) / 6.0f;
		}

		bReady = true;	// 3개의 B-Spline 곡선 함수 계수가 계산됨, display에서 시뮬레이션이 시작되게 만들음
		glfwSetTime(0.0f); // 시간을 0초로 초기화

		bClicked = false;
	}
}

#LINE 599 ~ 614
// 클릭이 초기 한 번 된 후에는, 소가 커서를 따라다니게 만들음
if (bClicked && bReady == false)
{
	Ray ray;
	screenCoordToRay(x, y, ray);
	PickInfo &pp = pickInfo;
	Plane p(vector3(0, 1, 0), pp.cowPickPosition);
	std::pair<bool, double> c = ray.intersects(p);

	vector3 currentPos = ray.getPoint(c.second);

	matrix4 R, T;
	R.setRotationY(-M_PI / 2);
	T.setTranslation(currentPos - pp.cowPickPosition, false);
	T *= R;
	cow2wld.mult(T, pp.cowPickConfiguration);
}