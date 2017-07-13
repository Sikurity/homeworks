This is an Lee YeongSik's Computer Graphic Practice Assignment Report
==========================================================================================================================================

�߰� �� ���� �ڵ� ���� �� �� ��ȣ
------------------------------------------------------------------------------------------------------------------------------------------
#LINE 48 ~ 59
int cntCow;					// ����ڰ� �����Ų �� ������Ʈ ���� ī��Ʈ

matrix4 dupCow[6];			// ����ڰ� Ctrl�� ���� ���縦 ������ dupCow[cntCow]�� cow2wld�� ����Ǿ� ����Ǵ� ����
vector3 prevCow;			// �ùķ��̼� �Ǵ� ����, �Ұ� �ٶ󺸴� ������ ����ϱ� ���� ���� ���� ��ġ�� �����ϱ� ���� ����

double bspCoefX[3][4];		// bspCoef[0] : 1��° B-Spline Curve, bspCoef[1] : 2��° B-Spline Curve, bspCoef[2] : 3��° B-Spline Curve
double bspCoefZ[3][4];		// bspCoef[n][0] : 3���� ���, bspCoef[n][1] : 2���� ���, bspCoef[n][2] : 1���� ���, bspCoef[n][3] : 0���� ���
double curTime;				// �ùķ��̼��� �̷����� ���� 0.0f ~ 3.0f ���� ��ȯ�ϴ� �ð� ����

bool bReady;				// ��ġ�� ��� �����Ǹ� ������ 6���� ���� ��ġ�� B-Spline �Լ��� ��� 3���� ���� ������ �� false -> true�� �ٲ�
bool bClicked;				// �Ҹ� �����ߴ��� ���θ� �����ϴ� ����
bool isCowReady[6];			// ����ڰ� Ctrl�� ���� ���縦 ������ isCowReady[cntCow] : false -> true�� �ٲ��, true�� ��� ���

#LINE 307 ~ 360
if( bReady )						// �� 6����, B-Spling � 3���� �ϼ��Ǹ� ����Ǵ� �б⹮
{
	curTime = glfwGetTime();			// ���� �ð��� �����´�

	// 0 ~ 3�� ���ȸ� �ùķ��̼� �ϴ� ���̹Ƿ�, 3�� �ʰ��� ��� ���� �ʱ�ȭ
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
		t_time = curTime - (int)curTime;		// ���� �ð����� �Ҽ��� �κи� ���, B-Spling ��� 0 <= t <= 1�� ���ؼ� ���ǵǱ� ����

		// Bspline �Լ��� ����鿡 ����ð� t^n�� ���� �����ش�, ������ �ּ�ȭ �ϱ� ���� �Ʒ��� ���� ������
		// ���� ��ǥ�� �ùķ��̼� �� �� ������Ʈ�� ��ġ ������ ����
		cowPos.x = bspCoefX[(int)curTime][3] + (bspCoefX[(int)curTime][2] + t_time * (bspCoefX[(int)curTime][1] + t_time * bspCoefX[(int)curTime][0])) * t_time;
		cowPos.y = 0.0f;
		cowPos.z = bspCoefZ[(int)curTime][3] + (bspCoefZ[(int)curTime][2] + t_time * (bspCoefZ[(int)curTime][1] + t_time * bspCoefZ[(int)curTime][0])) * t_time;

		// �ùķ��̼� �� �Ұ� �ٶ󺸴� ������ ���ϱ� ���Ͽ�, ���� ��ġ���� ���� ��ġ���� ���� ���͸� ����
		cowDir = cowPos - prevCow;
		
		// ������ ���� ���� ���Ϳ��� arctan�� �̿��� ȸ������ ���� y�࿡ ���� ȸ�� ��Ŵ, arctan�� ���� ��ȣ�� ���� ���� ������
		cow2wld.setRotationY(-atan2(cowDir.z, cowDir.x));
		cow2wld.setTranslation(cowPos);

		// ���� ��ġ�� ������ ���� ������ ���� ��ġ�� ����
		prevCow = cowPos;
	}
}
// ���� B-Spline�� ���� ���� ������ �����ϸ� ����Ǵ� �б⹮
else
{
	// ctrl�� ���� �� ���� �þ�� ����� �Ҹ� ȭ�鿡 ���
	for( i = 0 ; i < 6 ; i++ )
	{
		if( isCowReady[i] )
			drawCow(dupCow[i], cursorOnCowBoundingBox);
	}
}

#LINE 509
static double p_x[6], p_z[6];

#LINE 521 ~ 569
if (!bClicked && cursorOnCowBoundingBox)	// �Ҹ� ���ʷ� Ŭ�� �� ���, bClicked�� True�� �ٲ���
	bClicked = true;
else if (bClicked && bReady == false)	// Ctrl Ű�� ���Ȱ� && �ùķ��̼� ������ �ƴѰ��
{
	isCowReady[cntCow] = true;		// �Ұ� ����Ǿ����� Ȯ���ϴ� ����
	dupCow[cntCow++] = cow2wld;;	// CtrlŰ�� ���� ������ �� ������Ʈ�� ����

	// ����� �Ұ� 6�������, ���� ��ġ 6���� ��� �������ٴ� ���̹Ƿ�, B-Spline ��� �����Ѵ�
	if (cntCow == 6)
	{
		// �� 6���� ��ġ ����
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

		// B-Spline � �Լ��� 0, 1, 2, 3�� ��� ����
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

		bReady = true;	// 3���� B-Spline � �Լ� ����� ����, display���� �ùķ��̼��� ���۵ǰ� ������
		glfwSetTime(0.0f); // �ð��� 0�ʷ� �ʱ�ȭ

		bClicked = false;
	}
}

#LINE 599 ~ 614
// Ŭ���� �ʱ� �� �� �� �Ŀ���, �Ұ� Ŀ���� ����ٴϰ� ������
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