//#define _CRT_SECURE_NO_WARNINGS
//
//#include <iostream>
//#include <fstream>
//#include <stdlib.h>
//#include <string.h>
//#include <math.h>
//#include <sys/types.h>
//
//#ifdef _MSC_VER
//#include <Windows.h>
//#include <stdlib.h>
//#endif
//
//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glfw.h>
//#include "math/matrix4.h"
//#include "math/vector3.h"
//#include "math/intersectionTest.h"
//#include "WaveFrontOBJ.h"
//#include <assert.h>
//
//#ifndef M_PI
//#define M_PI 3.1415926535897932384626433832795
//#endif
//
//void screenCoordToRay(int x, int y, Ray& ray);
//
//// 'cameras' stores infomation of 5 cameras.
//double cameras[5][9] = 
//{
//	{28,18,28, 0,2,0, 0,1,0},   
//	{28,18,-28, 0,2,0, 0,1,0}, 
//	{-28,18,28, 0,2,0, 0,1,0}, 
//	{-12,12,0, 0,2,0, 0,1,0},  
//	{0,100,0,  0,0,0, 1,0,0}
//};
//int cameraCount = sizeof( cameras ) / sizeof( cameras[0] );
//
//int cameraIndex, camID;
//vector<matrix4> wld2cam, cam2wld; 
//WaveFrontOBJ* cam;
//
//// Variables for 'cow' object.
//matrix4 cow2wld;
//
//int cntCow;					// 사용자가 복사시킨 소 오브젝트 개수 카운트
//
//matrix4 dupCow[6];			// 사용자가 Ctrl을 눌러 복사를 누르면 dupCow[cntCow]에 cow2wld가 복사되어 저장되는 변수
//vector3 prevCow;			// 시뮬레이션 되는 동안, 소가 바라보는 방향을 계산하기 위해 이전 소의 위치를 저장하기 위한 변수
//
//double bspCoefX[3][4];		// bspCoef[0] : 1번째 B-Spline Curve, bspCoef[1] : 2번째 B-Spline Curve, bspCoef[2] : 3번째 B-Spline Curve
//double bspCoefZ[3][4];		// bspCoef[n][0] : 3차항 계수, bspCoef[n][1] : 2차항 계수, bspCoef[n][2] : 1차항 계수, bspCoef[n][3] : 0차항 계수
//double curTime;				// 시뮬레이션이 이뤄지는 동안 0.0f ~ 3.0f 으로 순환하는 시간 변수
//
//bool bReady;				// 위치가 모두 선정되면 선정된 6개의 점의 위치로 B-Spline 함수의 계수 3쌍을 구해 저장한 후 false -> true로 바뀜
//bool bClicked;				// 소를 선택했는지 여부를 저장하는 변수
//bool isCowReady[6];			// 사용자가 Ctrl을 눌러 복사를 누르면 isCowReady[cntCow] : false -> true로 바뀌고, true일 경우 출력
//
//WaveFrontOBJ* cow;
//int cowID;
//bool cursorOnCowBoundingBox;
//
//struct PickInfo
//{
//	double cursorRayT;
//	vector3 cowPickPosition;
//	matrix4 cowPickConfiguration; // backs up cow2wld
//};
//
//PickInfo pickInfo;
//
//// Variables for 'beethovan' object.
//matrix4 bet2wld;
//WaveFrontOBJ* bet;
//int betID;
//
//unsigned floorTexID;
//int width, height;
//int  oldX, oldY;
//
//void drawFrame(float len);
//void initialize();
//
//enum { L_DRAG=1, R_DRAG=2};
//static int isDrag=0;
////------------------------------------------------------------------------------
//
//Plane makePlane(vector3 const& a, vector3 const& b, vector3 const& n)
//{
//	vector3 v=a;
//	for(int i=0; i<3; i++)
//	{
//		if(n[i]==1.0)
//			v[i]=b[i];
//		else if(n[i]==-1.0)
//			v[i]=a[i];
//		else
//			assert(n[i]==0.0);
//	}
//	//std::cout<<n<<v<<std::endl;
//		
//	return Plane(cow2wld.rotate(n),cow2wld*v);
//}
//
////------------------------------------------------------------------------------
//void drawOtherCamera()
//{
//	int i;
//
//	// draw other cameras.
//	for (i=0; i < (int)wld2cam.size(); i++ )
//	{
//		if (i != cameraIndex)
//		{
//			glPushMatrix();												// Push the current matrix on GL to stack. The matrix is wld2cam[cameraIndex].matrix().
//			glMultMatrixd(cam2wld[i].GLmatrix());						// Multiply the matrix to draw i-th camera.
//			drawFrame(5);												// Draw x, y, and z axis.
//			float frontColor[] = {0.2, 0.2, 0.2, 1.0};
//			glEnable(GL_LIGHTING);									
//			glMaterialfv(GL_FRONT, GL_AMBIENT, frontColor);				// Set ambient property frontColor.
//			glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);				// Set diffuse property frontColor.
//			glScaled(0.5,0.5,0.5);										// Reduce camera size by 1/2.
//			glTranslated(1.1,1.1,0.0);									// Translate it (1.1, 1.1, 0.0).
//			glCallList(camID);											// Re-draw using display list from camID. 
//			glPopMatrix();												// Call the matrix on stack. wld2cam[cameraIndex].matrix() in here.
//		}
//	}
//}
//
///*********************************************************************************
//* Draw x, y, z axis of current frame on screen.
//* x, y, and z are corresponded Red, Green, and Blue, resp.
//**********************************************************************************/
//void drawFrame(float len)
//{
//	glDisable(GL_LIGHTING);		// Lighting is not needed for drawing axis.
//	glBegin(GL_LINES);			// Start drawing lines.
//	glColor3d(1,0,0);			// color of x-axis is red.
//	glVertex3d(0,0,0);			
//	glVertex3d(len,0,0);		// Draw line(x-axis) from (0,0,0) to (len, 0, 0). 
//	glColor3d(0,1,0);			// color of y-axis is green.
//	glVertex3d(0,0,0);			
//	glVertex3d(0,len,0);		// Draw line(y-axis) from (0,0,0) to (0, len, 0).
//	glColor3d(0,0,1);			// color of z-axis is  blue.
//	glVertex3d(0,0,0);
//	glVertex3d(0,0,len);		// Draw line(z-axis) from (0,0,0) - (0, 0, len).
//	glEnd();					// End drawing lines.
//}
//
///*********************************************************************************
//* Draw 'cow' object.
//**********************************************************************************/
//void drawCow(matrix4 const& _cow2wld, bool drawBB)
//{  
//	glPushMatrix();		// Push the current matrix of GL into stack. This is because the matrix of GL will be change while drawing cow.
//
//	// The information about location of cow to be drawn is stored in cow2wld matrix.
//	// (Project2 hint) If you change the value of the cow2wld matrix or the current matrix, cow would rotate or move.
//	glMultMatrixd(_cow2wld.GLmatrix());
//
//	drawFrame(5);										// Draw x, y, and z axis.
//	float frontColor[] = {0.8, 0.2, 0.9, 1.0};
//	glEnable(GL_LIGHTING);
//	glMaterialfv(GL_FRONT, GL_AMBIENT, frontColor);		// Set ambient property frontColor.
//	glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);		// Set diffuse property frontColor.
//	glCallList(cowID);		// Draw cow. 
//	glDisable(GL_LIGHTING);
//	
//	if(drawBB)
//	{
//		glBegin(GL_LINES);
//		glColor3d(1,1,1);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmax.z);
//
//		glColor3d(1,1,1);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmax.z);
//
//		glColor3d(1,1,1);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmax.z);
//
//
//		glColor3d(1,1,1);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmax.z);
//
//		glColor3d(1,1,1);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmin.y, cow->bbmax.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmin.x, cow->bbmax.y, cow->bbmax.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmin.z);
//		glVertex3d( cow->bbmax.x, cow->bbmax.y, cow->bbmax.z);
//		glEnd();
//	}
//
//	glPopMatrix();			// Pop the matrix in stack to GL. Change it the matrix before drawing cow.
//}
//
///*********************************************************************************
//* Draw 'beethovan' object.
//**********************************************************************************/
//void drawBet()
//{  
//	glPushMatrix();
//	glMultMatrixd(bet2wld.GLmatrix());
//	drawFrame(8);
//	float frontColor[] = {0.8, 0.3, 0.1, 1.0};
//	glEnable(GL_LIGHTING);
//	glMaterialfv(GL_FRONT, GL_AMBIENT, frontColor);
//	glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);
//	glCallList(betID);
//	glPopMatrix();
//}
//
//
///*********************************************************************************
//* Draw floor on 3D plane.
//**********************************************************************************/
//void drawFloor()
//{  
//	glPushMatrix();
//	glTranslated(0.0f, -5.0f, 0.0f);
//	glDisable(GL_LIGHTING);
//	// Set background color.
//	glColor3d(0.35, .2, 0.1);
//	// Draw background rectangle. 
//	glBegin(GL_POLYGON);
//	glVertex3f( 2000,-0.2, 2000);
//	glVertex3f( 2000,-0.2,-2000);
//	glVertex3f(-2000,-0.2,-2000);
//	glVertex3f(-2000,-0.2, 2000);
//	glEnd();
//
//	// Set color of the floor.
//	// Assign checker-patterned texture.
//	glEnable(GL_TEXTURE_2D);
//	glBindTexture(GL_TEXTURE_2D, floorTexID );
//
//	// Draw the floor. Match the texture's coordinates and the floor's coordinates resp. 
//	glBegin(GL_POLYGON);
//
//	glTexCoord2d(0,0);
//	glVertex3d(-12,-0.1,-12);		// Texture's (0,0) is bound to (-12,-0.1,-12).
//	glTexCoord2d(1,0);
//	glVertex3d( 12,-0.1,-12);		// Texture's (1,0) is bound to (12,-0.1,-12).
//	glTexCoord2d(1,1);
//	glVertex3d( 12,-0.1, 12);		// Texture's (1,1) is bound to (12,-0.1,12).
//	glTexCoord2d(0,1);
//	glVertex3d(-12,-0.1, 12);		// Texture's (0,1) is bound to (-12,-0.1,12).
//	glEnd();
//	glDisable(GL_TEXTURE_2D);
//	drawFrame(5);				// Draw x, y, and z axis.
//	glPopMatrix();
//}
//
//
///*********************************************************************************
//* Call this part whenever display events are needed. 
//* Display events are called in case of re-rendering by OS. ex) screen movement, screen maximization, etc.
//**********************************************************************************/
//void display()
//{
//	double t_time;
//	int i;
//	vector3 cowPos, cowDir;
//
//	glClearColor(0, 0.6, 0.8, 1);									// Clear color setting
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear the screen
//	// set viewing transformation.
//	glLoadMatrixd(wld2cam[cameraIndex].GLmatrix());
//
//	drawOtherCamera();												// Locate the camera's position, and draw all of them.
//	drawFloor();													// Draw floor.
//	//drawBet();
//
//	if( bReady )						// 점 6개로, B-Spling 곡선 3개가 완성되면 실행되는 분기문
//	{
//		curTime = glfwGetTime();			// 현재 시간을 가져온다
//
//		// 0 ~ 3초 동안만 시뮬레이션 하는 것이므로, 3초 초과시 모든 것을 초기화
//		if (curTime >= 3.0f)
//		{
//			cntCow = 0;
//			curTime = 0.0f;
//			bReady = false;
//
//			memset(isCowReady, 0, sizeof(isCowReady));
//			memset(dupCow, 0, sizeof(dupCow));
//			memset(bspCoefX, 0, sizeof(bspCoefX));
//			memset(bspCoefZ, 0, sizeof(bspCoefZ));
//
//			prevCow.x = 0.0f;
//			prevCow.y = 0.0f;
//			prevCow.z = 0.0f;
//
//			cow2wld.setTranslation(prevCow);
//			cow2wld.setRotationY(-M_PI / 2);
//		}
//		else
//		{
//			t_time = curTime - (int)curTime;		// 현재 시간에서 소수점 부분만 사용, B-Spling 곡선이 0 <= t <= 1에 대해서 정의되기 때문
//
//			// Bspline 함수의 계수들에 현재시간 t^n을 구해 곱해준다, 곱셈을 최소화 하기 위해 아래와 같이 곱해줌
//			// 구한 좌표를 시뮬레이션 할 소 오브젝트의 위치 값으로 대입
//			cowPos.x = bspCoefX[(int)curTime][3] + (bspCoefX[(int)curTime][2] + t_time * (bspCoefX[(int)curTime][1] + t_time * bspCoefX[(int)curTime][0])) * t_time;
//			cowPos.y = 0.0f;
//			cowPos.z = bspCoefZ[(int)curTime][3] + (bspCoefZ[(int)curTime][2] + t_time * (bspCoefZ[(int)curTime][1] + t_time * bspCoefZ[(int)curTime][0])) * t_time;
//
//			// 시뮬레이션 중 소가 바라보는 방향을 구하기 위하여, 직전 위치에서 현재 위치로의 방향 벡터를 구함
//			cowDir = cowPos - prevCow;
//		
//			// 위에서 구한 방향 벡터에서 arctan를 이용해 회전각을 구해 y축에 대해 회전 시킴, arctan에 음수 부호를 붙인 것은 보정용
//			cow2wld.setRotationY(-atan2(cowDir.z, cowDir.x));
//			cow2wld.setTranslation(cowPos);
//
//			// 이전 위치를 저장해 놓는 변수를 현재 위치로 갱신
//			prevCow = cowPos;
//		}
//	}
//	// 아직 B-Spline을 만들 점의 개수가 부족하면 실행되는 분기문
//	else
//	{
//		// ctrl을 누를 때 마다 늘어나는 복사된 소를 화면에 출력
//		for( i = 0 ; i < 6 ; i++ )
//		{
//			if( isCowReady[i] )
//				drawCow(dupCow[i], cursorOnCowBoundingBox);
//		}
//	}
//
//	drawCow(cow2wld, cursorOnCowBoundingBox);						// Draw cow.
//
//	glFlush();
//
//	glfwSwapBuffers();
//}
//
//
///*********************************************************************************
//* Call this part whenever size of the window is changed. 
//**********************************************************************************/
//void reshape( int w, int h)
//{
//	width = w;
//	height = h;
//	glViewport(0, 0, width, height);
//	glMatrixMode(GL_PROJECTION);            // Select The Projection Matrix
//	glLoadIdentity();                       // Reset The Projection Matrix
//	// Define perspective projection frustum
//	double aspect = width/double(height);
//	gluPerspective(45, aspect, 1, 1024);
//	glMatrixMode(GL_MODELVIEW);             // Select The Modelview Matrix
//	glLoadIdentity();                       // Reset The Projection Matrix
//}
//
////------------------------------------------------------------------------------
//void initialize()
//{
//	cursorOnCowBoundingBox=false;
//	// Set up OpenGL state
//	glShadeModel(GL_SMOOTH);         // Set Smooth Shading
//	glEnable(GL_DEPTH_TEST);         // Enables Depth Testing
//	glDepthFunc(GL_LEQUAL);          // The Type Of Depth Test To Do
//	// Use perspective correct interpolation if available
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
//	// Initialize the matrix stacks
//	reshape(width, height);
//	// Define lighting for the scene
//	float lightDirection[]   = {1.0, 1.0, 1.0, 0};
//	float ambientIntensity[] = {0.1, 0.1, 0.1, 1.0};
//	float lightIntensity[]   = {0.9, 0.9, 0.9, 1.0};
//	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientIntensity);
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
//	glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
//	glEnable(GL_LIGHT0);
//
//	// initialize floor
//	{
//		// After making checker-patterned texture, use this repetitively.
//
//		// Insert color into checker[] according to checker pattern.
//		const int size = 8;
//		unsigned char checker[size*size*3];
//		for( int i=0; i < size*size; i++ )
//		{
//			if (((i/size) ^ i) & 1)
//			{
//				checker[3*i+0] = 200;
//				checker[3*i+1] = 32;
//				checker[3*i+2] = 32;
//			}
//			else
//			{
//				checker[3*i+0] = 200;
//				checker[3*i+1] = 200;
//				checker[3*i+2] = 32;
//			}
//		}
//
//		// Make texture which is accessible through floorTexID. 
//		glGenTextures( 1, &floorTexID );				
//		glBindTexture(GL_TEXTURE_2D, floorTexID);		
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//		glTexImage2D(GL_TEXTURE_2D, 0, 3, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, checker);
//	}
//	// initialize cow
//	{
//		// Read information from cow.obj.
//		cow = new WaveFrontOBJ( "cow.obj" );
//
//		// Make display list. After this, you can draw cow using 'cowID'.
//		cowID = glGenLists(1);				// Create display lists
//		glNewList(cowID, GL_COMPILE);		// Begin compiling the display list using cowID
//		cow->Draw();						// Draw the cow on display list.
//		glEndList();						// Terminate compiling the display list. Now, you can draw cow using 'cowID'.
//		glPushMatrix();						// Push the current matrix of GL into stack.
//		glLoadIdentity();					// Set the GL matrix Identity matrix.
//		glTranslated(0,-cow->bbmin.y,-8);	// Set the location of cow.
//		glRotated(-90, 0, 1, 0);			// Set the direction of cow. These information are stored in the matrix of GL.
//		// Read the modelview matrix about location and direction set above, and store it in cow2wld matrix.
//		cow2wld.getCurrentOpenGLmatrix(	GL_MODELVIEW_MATRIX);
//
//		glPopMatrix();						// Pop the matrix on stack to GL.
//	}
//	// initialize bethoben
//	{
//		// Read information from beethovan.obj.
//		bet = new WaveFrontOBJ( "beethovan.obj" );
//
//		// Make display list. After this, you can draw beethovan using 'betID'.
//		betID = glGenLists(1);
//		glNewList(betID, GL_COMPILE);
//		bet->Draw();
//		glEndList();
//		glPushMatrix();
//		glLoadIdentity();
//		glTranslated(0,-bet->bbmin.y,8);
//		glRotated(180, 0, 1, 0);
//		// bet2wld will become T * R
//		bet2wld.getCurrentOpenGLmatrix(GL_MODELVIEW_MATRIX);
//		glPopMatrix();
//	}
//	// intialize camera model.
//	{
//		cam = new WaveFrontOBJ("camera.obj");	// Read information of camera from camera.obj.
//		camID = glGenLists(1);					// Create display list of the camera.
//		glNewList(camID, GL_COMPILE);			// Begin compiling the display list using camID.
//		cam->Draw();							// Draw the camera. you can do this job again through camID..
//		glEndList();							// Terminate compiling the display list.
//
//		// initialize camera frame transforms.
//		for (int i=0; i < cameraCount; i++ )
//		{
//			double* c = cameras[i];											// 'c' points the coordinate of i-th camera.
//			wld2cam.push_back(matrix4());								// Insert {0} matrix to wld2cam vector.
//			glPushMatrix();													// Push the current matrix of GL into stack.
//			glLoadIdentity();												// Set the GL matrix Identity matrix.
//			gluLookAt(c[0],c[1],c[2], c[3],c[4],c[5], c[6],c[7],c[8]);		// Setting the coordinate of camera.
//			wld2cam[i].getCurrentOpenGLmatrix(GL_MODELVIEW_MATRIX);
//			glPopMatrix();													// Transfer the matrix that was pushed the stack to GL.
//			matrix4 invmat;
//			invmat.inverse(wld2cam[i]);
//			cam2wld.push_back(invmat);
//		}
//		cameraIndex = 0;
//	}
//}
//
///*********************************************************************************
//* Call this part whenever mouse button is clicked.
//**********************************************************************************/
//void onMouseButton(int button, int state)
//{
//	static double p_x[6], p_z[6];
//
//	const int GLFW_DOWN=1;
//	const int GLFW_UP=0;
//
//   	int x, y, i;
//	glfwGetMousePos(&x, &y);
//
//	if (button == GLFW_MOUSE_BUTTON_LEFT)
//	{
//		if (state == GLFW_DOWN)
//		{
//			if (!bClicked && cursorOnCowBoundingBox)	// 소를 최초로 클릭 한 경우, bClicked를 True로 바꿔줌
//				bClicked = true;
//			else if (bClicked && bReady == false)	// Ctrl 키가 눌렸고 && 시뮬레이션 도중이 아닌경우
//			{
//				isCowReady[cntCow] = true;		// 소가 복사되었는지 확인하는 변수
//				dupCow[cntCow++] = cow2wld;;	// Ctrl키가 눌린 순간의 소 오브젝트를 복사
//
//				// 복사된 소가 6마리라면, 점의 위치 6개가 모두 정해졌다는 뜻이므로, B-Spline 곡선을 생성한다
//				if (cntCow == 6)
//				{
//					// 점 6개의 위치 저장
//					p_x[0] = dupCow[0].getTranslation().x;
//					p_z[0] = dupCow[0].getTranslation().z;
//
//					p_x[1] = dupCow[1].getTranslation().x;
//					p_z[1] = dupCow[1].getTranslation().z;
//					
//					p_x[2] = dupCow[2].getTranslation().x;
//					p_z[2] = dupCow[2].getTranslation().z;
//
//					p_x[3] = dupCow[3].getTranslation().x;
//					p_z[3] = dupCow[3].getTranslation().z;
//
//					p_x[4] = dupCow[4].getTranslation().x;
//					p_z[4] = dupCow[4].getTranslation().z;
//
//					p_x[5] = dupCow[5].getTranslation().x;
//					p_z[5] = dupCow[5].getTranslation().z;
//
//					// B-Spline 곡선 함수의 0, 1, 2, 3차 계수 생성
//					for (i = 0; i < 3; i++)
//					{
//						bspCoefX[i][0] = (-p_x[i] + 3 * p_x[i + 1] - 3 * p_x[i + 2] + p_x[i + 3]) / 6.0f;
//						bspCoefX[i][1] = (3 * p_x[i] - 6 * p_x[i + 1] + 3 * p_x[i + 2]) / 6.0f;
//						bspCoefX[i][2] = (-3 * p_x[i] + 3 * p_x[i + 2]) / 6.0f;
//						bspCoefX[i][3] = (p_x[i] + 4 * p_x[i + 1] + p_x[i + 2]) / 6.0f;
//
//						bspCoefZ[i][0] = (-p_z[i] + 3 * p_z[i + 1] - 3 * p_z[i + 2] + p_z[i + 3]) / 6.0f;
//						bspCoefZ[i][1] = (3 * p_z[i] - 6 * p_z[i + 1] + 3 * p_z[i + 2]) / 6.0f;
//						bspCoefZ[i][2] = (-3 * p_z[i] + 3 * p_z[i + 2]) / 6.0f;
//						bspCoefZ[i][3] = (p_z[i] + 4 * p_z[i + 1] + p_z[i + 2]) / 6.0f;
//					}
//
//					bReady = true;	// 3개의 B-Spline 곡선 함수 계수가 계산됨, display에서 시뮬레이션이 시작되게 만들음
//					glfwSetTime(0.0f); // 시간을 0초로 초기화
//
//					bClicked = false;
//				}
//			}
//
//			isDrag=L_DRAG;
//			printf( "Left mouse click at (%d, %d)\n", x, y );
//		}
//		else
//			isDrag = 0;
//	}
//	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
//	{
//		if (state == GLFW_DOWN)
//		{
//			isDrag=R_DRAG;
//			printf( "Right mouse click at (%d, %d)\n",x,y );
//		}
//		else
//			isDrag=0;
//	}
//}
//
///*********************************************************************************
//* Call this part whenever user drags mouse. 
//* Input parameters x, y are coordinate of mouse on dragging. 
//* Value of global variables oldX, oldY is stored on onMouseButton, 
//* Then, those are used to verify value of x - oldX,  y - oldY to know its movement.
//**********************************************************************************/
//void onMouseDrag( int x, int y)
//{
//	static int prev_x = -1;
//	
//	// 클릭이 초기 한 번 된 후에는, 소가 커서를 따라다니게 만들음
//	if (bClicked && bReady == false)
//	{
//		Ray ray;
//		screenCoordToRay(x, y, ray);
//		PickInfo &pp = pickInfo;
//		Plane p(vector3(0, 1, 0), pp.cowPickPosition);
//		std::pair<bool, double> c = ray.intersects(p);
//
//		vector3 currentPos = ray.getPoint(c.second);
//
//		matrix4 R, T;
//		R.setRotationY(-M_PI / 2);
//		T.setTranslation(currentPos - pp.cowPickPosition, false);
//		T *= R;
//		cow2wld.mult(T, pp.cowPickConfiguration);
//	}
//	else
//	{
//		if (isDrag)
//		{
//			printf("in %d drag (%d, %d)\n", isDrag, x - oldX, y - oldY);
//			if (isDrag == R_DRAG)
//			{
//				if (prev_x == -1)
//					prev_x = oldX;
//
//				matrix4 R;
//				R.setRotationX((x - prev_x)*0.01);
//				cow2wld *= R;
//			}
//			else
//			{
//				if (cursorOnCowBoundingBox)
//				{
//					Ray ray;
//					screenCoordToRay(x, y, ray);
//					PickInfo &pp = pickInfo;
//					Plane p(vector3(0, 1, 0), pp.cowPickPosition);
//					std::pair<bool, double> c = ray.intersects(p);
//
//					vector3 currentPos = ray.getPoint(c.second);
//
//					matrix4 T;
//					T.setTranslation(currentPos - pp.cowPickPosition, false);
//					cow2wld.mult(T, pp.cowPickConfiguration);
//				}
//			}
//		}
//		else
//		{
//			Ray ray;
//			screenCoordToRay(x, y, ray);
//
//			std::vector<Plane> planes;
//			vector3 bbmin(cow->bbmin.x, cow->bbmin.y, cow->bbmin.z);
//			vector3 bbmax(cow->bbmax.x, cow->bbmax.y, cow->bbmax.z);
//
//			planes.push_back(makePlane(bbmin, bbmax, vector3(0, 1, 0)));
//			planes.push_back(makePlane(bbmin, bbmax, vector3(0, -1, 0)));
//			planes.push_back(makePlane(bbmin, bbmax, vector3(1, 0, 0)));
//			planes.push_back(makePlane(bbmin, bbmax, vector3(-1, 0, 0)));
//			planes.push_back(makePlane(bbmin, bbmax, vector3(0, 0, 1)));
//			planes.push_back(makePlane(bbmin, bbmax, vector3(0, 0, -1)));
//
//			std::pair<bool, double> o = ray.intersects(planes);
//			cursorOnCowBoundingBox = o.first;
//			PickInfo &pp = pickInfo;
//			pp.cursorRayT = o.second;
//		}
//	}
//
//	prev_x = x;
//}
//
///*********************************************************************************
//* Call this part whenever user types keyboard. 
//**********************************************************************************/
//void onKeyPress( int key, int action)
//{
//	if (action==GLFW_RELEASE)
//		return 	; // do nothing
//	// If 'c' or space bar are pressed, alter the camera.
//	// If a number is pressed, alter the camera corresponding the number.
//	if ((key == ' ') || (key == 'c'))
//	{    
//		printf( "Toggle camera %d\n", cameraIndex );
//		cameraIndex += 1;
//	}
//	else if ((key >= '0') && (key <= '9'))
//		cameraIndex = key - '0';
//
//	if (cameraIndex >= (int)wld2cam.size())
//		cameraIndex = 0;
//}
//void screenCoordToRay(int x, int y, Ray& ray)
//{
//	int height , width;
//	glfwGetWindowSize(&width, &height);
//
//	matrix4 matProjection;
//	matProjection.getCurrentOpenGLmatrix(GL_PROJECTION_MATRIX);
//	matProjection*=wld2cam[cameraIndex];
//	matrix4 invMatProjection;
//	invMatProjection.inverse(matProjection);
//
//	vector3 vecAfterProjection, vecAfterProjection2;
//	// -1 <= v.x < 1 when 0 <= x < width
//	// -1 <= v.y < 1 when 0 <= y < height
//	vecAfterProjection.x = ((double)(x - 0)/(double)width)*2.0-1.0;
//	vecAfterProjection.y = ((double)(y - 0)/(double)height)*2.0-1.0;
//	vecAfterProjection.y*=-1;
//	vecAfterProjection.z = -10;
//
//	//std::cout<<"cowPosition in clip coordinate (NDC)"<<matProjection*cow2wld.getTranslation()<<std::endl;
//	
//	vector3 vecBeforeProjection=invMatProjection*vecAfterProjection;
//
//	// camera position
//	ray.origin()=cam2wld[cameraIndex].getTranslation();
//	ray.direction()=vecBeforeProjection-ray.origin();
//	ray.direction().normalize();
//
//	//std::cout<<"dir" <<ray.direction()<<std::endl;
//
//}
//
////------------------------------------------------------------------------------
//int main( int argc, char* argv[] )
//{
//	width = 800;
//	height = 600;
//	int BPP=32;
//
//	glfwInit(); // Initialize openGL.
//	// Create a window (8-bit depth-buffer, no alpha and stencil buffers, windowed)
//   	glfwOpenWindow(width, height, BPP/4, BPP/4, BPP/4, 1, 8, 1, GLFW_WINDOW) ;
//	glfwSetWindowTitle("Simple Scene");				// Make window whose name is "Simple Scene".
//	int rv,gv,bv;
//	glGetIntegerv(GL_RED_BITS,&rv);					// Get the depth of red bits from GL.
//	glGetIntegerv(GL_GREEN_BITS,&gv);				// Get the depth of green bits from GL.
//	glGetIntegerv(GL_BLUE_BITS,&bv);				// Get the depth of blue bits from GL.
//	printf( "Pixel depth = %d : %d : %d\n", rv, gv, bv );
//	initialize();									// Initialize the other thing.
//
//	cntCow = 0;
//	curTime = 0.0f;
//	bReady = false;
//
//	memset(isCowReady,	0, sizeof(isCowReady));
//	memset(dupCow,		0, sizeof(dupCow));
//	memset(bspCoefX,	0, sizeof(bspCoefX));
//	memset(bspCoefZ,	0, sizeof(bspCoefZ));
//
//	prevCow.x = 0.0f;
//	prevCow.y = 0.0f;
//	prevCow.z = 0.0f;
//		
//	cow2wld.setTranslation(prevCow);
//	cow2wld.setRotationY(-M_PI/2);
//
//	glfwSetKeyCallback(onKeyPress);					// Register onKeyPress function to call that when user presses the keyboard.
//	glfwSetMouseButtonCallback(onMouseButton);		// Register onMouseButton function to call that when user moves mouse.
//	glfwSetMousePosCallback(onMouseDrag);			// Register onMouseDrag function to call that when user drags mouse.
//
//	while(glfwGetWindowParam(GLFW_OPENED) )
//		display();
//
//	return 1;
//}

#include <GL/freeglut.h>
#include <GL/glu.h>
#include <GL/gl.h>

static int Day = 0, Time = 0;

void MyDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -2.0);
	/***지구***/
	glPushMatrix();
	//지구의공전
	glRotatef((float)Day, 0.0, 1.0, 0.0);
	glTranslatef(0.7, 0.0, 0.0);
	//지구의자전
	glRotatef((float)Time, 0.0, 1.0, 0.0);
	glColor3f(0.5, 0.6, 0.7);
	glutWireSphere(0.1, 30, 8);
	/***달***/
	glPushMatrix();
	//달의공전	
	glRotatef((float)Time, 0.0, 1.0, 0.0);
	glTranslatef(0.2, 0.0, 0.0);
	glColor3f(0.9, 0.8, 0.2);
	glutSolidSphere(0.04, 30, 8);
	glPopMatrix();
	glPopMatrix();
	/***태양***/
	glColor3f(1.0, 0.3, 0.4);
	glutSolidSphere(0.2, 30, 16);
	glutSwapBuffers();
}
void MyTimer(int value)
{
	Day = (Day + 10) % 360;
	Time = (Time + 15) % 360;
	glutTimerFunc(100, MyTimer, 1);
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Solar system");
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 3.0);	glutDisplayFunc(MyDisplay);
	glutTimerFunc(100, MyTimer, 1);
	glutMainLoop();
	return 0;
}
