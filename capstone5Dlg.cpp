
/*
* 2023년 전자공학부 캡스톤 
  팀명 : 상부상조(5조)
  주제 : 인공지능 기반 보행자 안전 관리 시스템

  프로그램 버전 : YoloV4, OPENCV_3.2, VisualStudio2019

  인식 가능 객체 : Victim : 사고자 | Crouch : 앉은 사람 | Warking : 보행자
				   Car : 차량 | Motorcycle : 오토바이

  내용 : 비디오 뷰어와 웹캠을 활용하여 5가지의 객체를 인식하여 보행자와 차량의 안전을 보조하는
		 프로그램으로 사고 발생시 사고 장면을 인식하고 최초 1회 캡처하여 차량 신호와 횡단보도
		 신호를 통제하고 텍스트 간판 등에 사고 발생 등을 알리며 캡처한 사진과 위치를 첨부하여
		 이메일을 통해 해당기관에 자동으로 신고하는 안전 관리 보조 프로그램을 제작하였습니다.

  Github : https://github.com/JUVING/Pedestrian-Safety-Management-System./blob/main/README.md
  email : nohjaemin99@gmail.com

*/

#include "pch.h"
#include "framework.h"
#include "capstone5.h"
#include "capstone5Dlg.h"
#include "afxdialogex.h"
#include <windows.h>
#include <thread>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")
#define new DEBUG_NEW
//using namespace std;

Ccapstone5Dlg::Ccapstone5Dlg(CWnd* pParent)
	: CDialogEx(IDD_CAPSTONE5_DIALOG, pParent)
	, m_str_person(_T("날씨 : 맑음 | 미세먼지 : 보통"))
	, m_str_car(_T("날씨 : 맑음 | 미세먼지 : 보통"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Ccapstone5Dlg::PlayAudio(const CString& audioFilePath)
{
	if (PlaySound(audioFilePath, NULL, SND_FILENAME | SND_ASYNC) == FALSE) {
		// Failed to play audio
		AfxMessageBox(_T("Failed to play audio."));
	}
}

void Ccapstone5Dlg::Ccapstone5Dlg::sign(int n)
{
	switch (n)
	{
	case 0: //사고발생
		m_str_person = _T("사고 발생 (119안전신고센터 : 119)"); // 변수 초기화
		m_str_car = _T("사고 발생 (119안전신고센터 : 119)"); // 변수 초기화
		if (!playaudio1)
		{
			PlayAudio(_T("사고 발생 음성"));

			playaudio1 = true;
		}
		break;
	case 1: //차량 및 보행자 진입
		m_str_person = _T("차량이 진입 중 주의 하세요"); // 변수 초기화
		m_str_car = _T("보행자 발견 주의 하세요"); // 변수 초기화
		if (!playaudio2)
		{
			PlayAudio(_T("차량 진입 음성"));

			playaudio2 = true;
		}
		break;
	case 2: //평시
		m_str_person = _T("날씨 : 맑음 | 미세먼지 : 보통"); // 변수 초기화
		m_str_car = _T("날씨 : 맑음 | 미세먼지 : 보통"); // 변수 초기화
		break;
	default:
		m_str_person = _T("날씨 : 맑음 | 미세먼지 : 보통"); // 변수 초기화
		m_str_car = _T("날씨 : 맑음 | 미세먼지 : 보통"); // 변수 초기화
		break;
	}
}
void  Ccapstone5Dlg::Captured()
{
	using std::string;
	using std::cout;
	using std::endl;
	Mat frame; // 비디오 이미지 1장을 OpenCV 포맷으로 저장하는 버퍼
	int count_frames = 0;
	
	if (!is_captured)
	{
		time_t now = time(nullptr);
		tm time_struct;
		localtime_s(&time_struct, &now);
		char buffer[80];
		strftime(buffer, 80, "%Y-%m-%d %H-%M-%S", &time_struct);

		
		string save_path = "캡처 저장 경로";

		imwrite(save_path, frame);
		cout << "Captured frame " << count_frames << endl;

		
		LoadImages();
		is_captured = true;
	}
		
}

void Ccapstone5Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_IMAGE, m_list_image);
	DDX_Text(pDX, IDC_STATIC_PERSON, m_str_person);
	DDX_Text(pDX, IDC_STATIC_CAR, m_str_car);
	DDX_Control(pDX, IDC_BMP1, m_image_traffic);
}

BEGIN_MESSAGE_MAP(Ccapstone5Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN_VIDEO, &Ccapstone5Dlg::OnBnClickedButtonOpenVideo)
	ON_BN_CLICKED(IDC_BUTTON_WEBCAM, &Ccapstone5Dlg::OnBnClickedButtonWebcam)
	ON_BN_CLICKED(IDC_BUTTON_ACCEPT, &Ccapstone5Dlg::OnBnClickedButtonAccept)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &Ccapstone5Dlg::OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDCANCEL, &Ccapstone5Dlg::OnBnClickedCancel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_IMAGE, &Ccapstone5Dlg::OnLvnItemchangedListImage)
	ON_WM_TIMER()

END_MESSAGE_MAP()

BOOL Ccapstone5Dlg::OnInitDialog()
{
	using std::string;
	CDialogEx::OnInitDialog();
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, path, MAX_PATH); // "...\Exe\Evaluation.exe" 경로
	CString msg(path);
	int n = msg.ReverseFind('\\'); // ...\Exe\ 까지 위치, 0,1,...n
	m_strHome = msg.Left(n + 1); // ...\Exe\ 까지 문자열복사, 좌측에서 (n+1)개 
	CString str_cfg = m_strHome + L"cfg 경로";
	CString str_wgt = m_strHome + L"weights 경로";
	int gpu_id = 0;

	// YOLOv4 라이브러리 객체 생성
	m_pDetector = new Detector(string(CT2CA(str_cfg)),string(CT2CA(str_wgt)));
	
	// 클래스 이름 목록 파일 읽기
	LoadClassName(m_strHome + L"객체 이름");

	// 영상파일 목록 - List Control 초기화
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);         // 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);        // 작은 아이콘을 설정합니다.

	// 영상파일 목록 - List Control 초기화
	LV_COLUMN lvColumn;
	wchar_t* list[1] = { _T("신고 목록") };
	int nWidth[1] = { 360 };
	for (int i = 0; i < 1; i++)
	{
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT; // CENTER;
		lvColumn.pszText = list[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = nWidth[i];
		m_list_image.InsertColumn(i, &lvColumn);
	}
	m_list_image.SetExtendedStyle(m_list_image.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 해당 경로에서 파일 목록 가져오기
	CString strFolderPath = _T("신고 목록 경로");
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strFolderPath + _T("\\*.*"));
	int nItemIndex = 0;
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots())
			continue;
		CString strFileName = finder.GetFileName();
		if (finder.IsDirectory())
			continue;

		// 파일 목록을 List Control에 추가하기
		m_list_image.InsertItem(nItemIndex, strFileName);
		nItemIndex++;
	}

	m_timerId1 = SetTimer(0, 20000, nullptr); //20초
	m_timerId2 = SetTimer(0, 300000, nullptr); //5분
	m_timerId3 = SetTimer(0, 5000, nullptr); //5초

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void Ccapstone5Dlg::OnTimer(UINT_PTR nIDEvent) 
{
		if (nIDEvent == m_timerId1)
		{
			KillTimer(m_timerId1); 
			is_captured = false;
			Captured();
		}
		else if (nIDEvent == m_timerId2)
		{
			KillTimer(m_timerId1); 
			is_captured = false;
			Captured();
		}
		else if (nIDEvent == m_timerId3)
		{
			KillTimer(m_timerId1);
			is_captured = false;
			//Captured();
		}
		
	CDialogEx::OnTimer(nIDEvent);
}

BOOL Ccapstone5Dlg::LoadClassName(const CString strFilePath)
{
	m_vecClassName.clear();
	wchar_t buffer[125];
	// coco.names -> UTF-8 decoding
	FILE* stream = _wfopen(strFilePath, L"rt+,ccs=UTF-8");
	if (stream == NULL) {
		AfxMessageBox(L"Can't open " + strFilePath);
		return false;
	}
	// 텍스트를 한 줄씩 읽어서 vector에 저장한다.
	// 한 줄 읽으면 끝에 dummy 문자추가 된 것 제거해야 한다.
	while (fgetws(buffer, 125, stream) != NULL) {
		CString strClass(buffer);
		CString strOnlyChar = strClass.Left(strClass.GetLength() - 1);
		m_vecClassName.push_back(strOnlyChar);
	}
	fclose(stream);
	return TRUE;
}


std::vector<bbox_t> Ccapstone5Dlg::DETECTOR(cv::Mat cv_image)
{
	using std::vector;
	vector<bbox_t> obj = m_pDetector->detect(cv_image, 0.45); //(cv_image, 0.45) 여기를 수정하면 임계치값(threshhold) 수정
	return obj;
}

float bboxOverlap(const cv::Rect& box1, const cv::Rect& box2)
{
	int x1 = std::max(box1.x, box2.x);
	int y1 = std::max(box1.y, box2.y);
	int x2 = std::min(box1.x + box1.width, box2.x + box2.width);
	int y2 = std::min(box1.y + box1.height, box2.y + box2.height);
	int w = std::max(0, x2 - x1);
	int h = std::max(0, y2 - y1);
	float overlap = (float)(w * h) / (box1.width * box1.height + box2.width * box2.height - w * h);
	return overlap;
}


void Ccapstone5Dlg::DrawObjectInformation(std::vector<bbox_t>& objects, cv::Mat& image)
{
	const float overlapThreshold = 0.3f; // IOU 임계값

	std::vector<bool> kept(objects.size(), true);

	for (int i = 0; i < objects.size(); i++)
	{
		if (!kept[i]) continue; // 이미 다른 박스와 합쳐진 객체에 대해서는 skip

		int linethickness = 3;
		cv::Scalar boxcolor = (objects[i].obj_id == 0) ? cv::Scalar(0, 0, 255) : (objects[i].obj_id == 1 ? cv::Scalar(0, 165, 255) :
			(objects[i].obj_id == 2) ? cv::Scalar(0, 255, 0) : (objects[i].obj_id == 3) ? cv::Scalar(255, 0, 0) :
			(objects[i].obj_id == 4) ? cv::Scalar(255, 0, 255) : cv::Scalar(0, 0, 0));

		cv::Rect box1(cv::Point(objects[i].x, objects[i].y), cv::Point(objects[i].x + objects[i].w, objects[i].y + objects[i].h));

		
		CString strName = m_vecClassName[objects[i].obj_id];
		std::string stringText = std::string(CT2CA(strName.operator LPCWSTR()));
		int fontface = 0;
		double fontscale = 0.7;
		int fontthickness = 2;
		int baseline = 0;
		cv::Size text = cv::getTextSize(stringText, fontface, fontscale, fontthickness, &baseline);

		cv::rectangle(image, cv::Point(objects[i].x, objects[i].y), cv::Point(objects[i].x + objects[i].w, objects[i].y + objects[i].h),
			(objects[i].obj_id == 0) ? cv::Scalar(0, 0, 255) : (objects[i].obj_id == 1 ? cv::Scalar(0, 165, 255) : (objects[i].obj_id == 2) ? cv::Scalar(0, 255, 0) :
				(objects[i].obj_id == 3) ? cv::Scalar(255, 0, 0) : (objects[i].obj_id == 4) ? cv::Scalar(255, 0, 255) : cv::Scalar(0, 0, 0)), linethickness);

		cv::rectangle(image, cv::Rect(box1.x, box1.y - text.height, text.width, text.height),
			(objects[i].obj_id == 0) ? cv::Scalar(0, 0, 255) : (objects[i].obj_id == 1 ? cv::Scalar(0, 165, 255) :
				(objects[i].obj_id == 2) ? cv::Scalar(0, 255, 0) : (objects[i].obj_id == 3) ? cv::Scalar(255, 0, 0) :
				(objects[i].obj_id == 4) ? cv::Scalar(255, 0, 255) : cv::Scalar(0, 0, 0)), CV_FILLED);

		cv::putText(image, stringText, cv::Point(box1.x, box1.y), fontface, fontscale, cv::Scalar(255, 255, 255), fontthickness, cv::LINE_AA);

		for (int j = i + 1; j < objects.size(); j++)
		{
			//if (!kept[j]) continue; // 이미 다른 박스와 합쳐진 객체에 대해서는 skip

			cv::Rect box2(cv::Point(objects[j].x, objects[j].y), cv::Point(objects[j].x + objects[j].w, objects[j].y + objects[j].h));

			float iou = bboxOverlap(box1, box2);
			if (iou > overlapThreshold)
			{
				// 같은 객체에 속하는 박스를 합쳐줌
				kept[j] = false;
				box1 = box1 | box2;
			}
		}
	}
}

void Ccapstone5Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR Ccapstone5Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString Ccapstone5Dlg::GetVideoFileName()
{
	CFileDialog dlg(TRUE, _T("All(*.*)"), _T("*.*"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("All(*.*)|*.*|(*.mp4)|*.mp4|(*.mov)|*.mov|(*.avi)|*.avi|"));
	if (dlg.DoModal() == IDOK)
	{
		CString strVideoFilePathName = dlg.GetPathName(); // 경로명+파일이름+확장자
		CString strFileName = dlg.GetFileName(); // 파일이름 + 확장자
		m_str_message.Format(L"%s ", strFileName);
		UpdateData(false); // variable -> control
		return strVideoFilePathName;
	}
}



void Ccapstone5Dlg::DisplayVideoStream(CString strVideoFileName)
{
	m_isVideoPlaying = true; //영상 실행 유무
	// Convert CString to std::string
	CT2CA convertedString(strVideoFileName);
	std::string s = std::string(convertedString);
	Mat frame; // 비디오 이미지 1장을 OpenCV 포맷으로 저장하는 버퍼
	VideoCapture cap(s); // 비디오 파일을 오픈한다.
	if (cap.isOpened() == false) {
		AfxMessageBox(strVideoFileName + L"\nCannot open the video file");
		return;
	}
	int w = cap.get(cv::CAP_PROP_FRAME_WIDTH); // 비디오 이미지 너비
	int h = cap.get(cv::CAP_PROP_FRAME_HEIGHT); // 비디오 이미지 높이
	double fps = cap.get(cv::CAP_PROP_FPS); // 초당 프레임 수
	int total_frames = cap.get(cv::CAP_PROP_FRAME_COUNT); // 총 프레임 수
	int count_frames = 0;
	bool is_captured = false;
	CString msg_head = m_str_message; // 파일 정보(파일 이름, 너비, 높이)

	while (true)
	{
		bool bSuccess = cap.read(frame); // 비디오에서 프레임 읽기
		if (bSuccess == false)
		{
			// 비디오 끝에서 루프 종료
			break;
		}

		// -------------------------------------------
		// YOLOv4 객체 탐지 함수 호출
		std::vector<bbox_t> objects = DETECTOR(frame);
		DrawObjectInformation(objects, frame);


		std::chrono::steady_clock::time_point start_time; // Variable to store the start time
		bool is_timer_started = false;
		
		// Iterate through detected objects
		for (auto obj : objects)
		{
			// Check if object ID is 0
			if (obj.obj_id == 0)
			{

				sign(obj.obj_id);

				CBitmap bmp;
				bmp.LoadBitmapW(IDB_BITMAP1);

				Captured();//캡처에 사용하는 함수
				
			}
			else if (obj.obj_id == 2 || obj.obj_id == 3 && obj.obj_id == 2 || obj.obj_id == 4)
			{//오브젝트 아이디 1, 2번(각 앉은 사람, 보행자)가 3, 4번(각 차량, 오토바이)가 같이 인식하는지 확인하는 조건문
	
				sign(1);
				
				CBitmap bmp;
				bmp.LoadBitmapW(IDB_BITMAP2);
				m_image_traffic.SetBitmap((HBITMAP)bmp.Detach()); //정상 신호
			}
			else if (obj.obj_id == 1 || obj.obj_id == 3 && obj.obj_id == 1 || obj.obj_id == 4)
			{
				sign(1);

				CBitmap bmp;
				bmp.LoadBitmapW(IDB_BITMAP2);
				m_image_traffic.SetBitmap((HBITMAP)bmp.Detach()); //정상 신호
			}
			else
			{
				sign(2);

				GetDlgItem(IDC_STATIC_PERSON)->SetWindowText(m_str_person); // static Text 컨트롤과 변수 연결
				GetDlgItem(IDC_STATIC_CAR)->SetWindowText(m_str_car); // static Text 컨트롤과 변수 연결
			}
		}
		// -------------------------------------------

		::imshow("보행자 안전 관리 시스템", frame); // 화면에 비디오 이미지 출력
		frame.release();
		CString msg;
		msg.Format(L"\n(w=%d h=%d %.2f fps) frame cnt= %d:(%d)",
			w, h, fps, ++count_frames, total_frames);
		m_str_message = msg_head + msg;
		UpdateData(false); // variable -> control
		if (waitKey(10) == 27)  // {Esc}키를 치면 종료 
		{
			m_isVideoPlaying = false; //영상 실행 유무

			m_list_image.DeleteAllItems();
			LoadImages();
			break;
		}
	}
	sign(2);

	GetDlgItem(IDC_STATIC_PERSON)->SetWindowText(m_str_person); // static Text 컨트롤과 변수 연결
	GetDlgItem(IDC_STATIC_CAR)->SetWindowText(m_str_car); // static Text 컨트롤과 변수 연결

	CBitmap bmp;
	bmp.LoadBitmapW(IDB_BITMAP2);
	m_image_traffic.SetBitmap((HBITMAP)bmp.Detach());

	cap.release();
}

void Ccapstone5Dlg::DisplayWebcamImage()
{
	Mat frame; // 버퍼로 사용할 비디오 이미지 1장을 OpenCV 포맷으로 저장
	VideoCapture cap(1); // 웹캠을 초기화한다. 내장 웹캠(0), 외장 웹캠(1)
	if (cap.isOpened() == false) {
		AfxMessageBox(L"Can not open Webcam!!");
		return;
	}
	int w = cap.get(cv::CAP_PROP_FRAME_WIDTH); // 비디오 이미지 너비
	int h = cap.get(cv::CAP_PROP_FRAME_HEIGHT); // 비디오 이미지 높이
	double fps = cap.get(cv::CAP_PROP_FPS); // 초당 프레임 수
	int total_frames = cap.get(cv::CAP_PROP_FRAME_COUNT); // 총 프레임 수
	int count_frames = 0;
	bool is_captured = false;
	CString msg_head = m_str_message; // 파일 정보(파일 이름, 너비, 높이)

	while (true)
	{
		bool bSuccess = cap.read(frame); // 비디오에서 프레임 읽기
		if (bSuccess == false)
		{
			// 비디오 끝에서 루프 종료
			break;
		}

		// -------------------------------------------
		// YOLOv4 객체 탐지 함수 호출
		std::vector<bbox_t> objects = DETECTOR(frame);
		DrawObjectInformation(objects, frame);

		for (auto obj : objects)
		{
			//오브젝트 아이디 0인지 확인
			if (obj.obj_id == 0)
			{
				sign(obj.obj_id);

				CBitmap bmp;
				bmp.LoadBitmapW(IDB_BITMAP1);
				sign(0);
				//m_image_traffic.SetBitmap((HBITMAP)bmp.Detach()); // 비상 신호로 변경

				Captured(); //캡처에 사용하는 함수
			}
			else if (obj.obj_id == 2 || obj.obj_id == 3 && obj.obj_id == 2 || obj.obj_id == 4)
			{
				sign(1);

				CBitmap bmp;
				bmp.LoadBitmapW(IDB_BITMAP2);
				//m_image_traffic.SetBitmap((HBITMAP)bmp.Detach()); //정상 신호
			}
			else if (obj.obj_id == 1 || obj.obj_id == 3 && obj.obj_id == 1 || obj.obj_id == 4)
			{
				sign(1);

				CBitmap bmp;
				bmp.LoadBitmapW(IDB_BITMAP2);
				//m_image_traffic.SetBitmap((HBITMAP)bmp.Detach()); //정상 신호
			}
			else
			{
				sign(0);

				GetDlgItem(IDC_STATIC_PERSON)->SetWindowText(m_str_person); // static Text 컨트롤과 변수 연결
				GetDlgItem(IDC_STATIC_CAR)->SetWindowText(m_str_car); // static Text 컨트롤과 변수 연결
			}
		}
		// -------------------------------------------

		::imshow("보행자 안전 관리 시스템", frame); // 화면에 비디오 이미지 출력
		frame.release();
		CString msg;
		msg.Format(L"\n(w=%d h=%d %.2f fps) frame cnt= %d:(%d)",
			w, h, fps, ++count_frames, total_frames);
		m_str_message = msg_head + msg;
		UpdateData(false); // variable -> control
		if (waitKey(10) == 27)  // {Esc}키를 치면 종료 
		{
			m_isVideoPlaying = false; //영상 실행 유무

			m_list_image.DeleteAllItems();
			LoadImages();
			break;
		}
	}
	sign(0);

	GetDlgItem(IDC_STATIC_PERSON)->SetWindowText(m_str_person); // static Text 컨트롤과 변수 연결
	GetDlgItem(IDC_STATIC_CAR)->SetWindowText(m_str_car); // static Text 컨트롤과 변수 연결

	CBitmap bmp;
	bmp.LoadBitmapW(IDB_BITMAP2);

	cap.release();
}

void Ccapstone5Dlg::OnLvnItemchangedListImage(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;
}

void Ccapstone5Dlg::OnBnClickedButtonOpenVideo()
{
	CString strVideoFileName = GetVideoFileName();
	DisplayVideoStream(strVideoFileName);
}


void Ccapstone5Dlg::OnBnClickedButtonWebcam()
{
	DisplayWebcamImage();
}

void Ccapstone5Dlg::OnBnClickedButtonAccept()
{
	CString filePath = _T("이메일 전송 프로그램 경로");
	//이메일 전송 프로그램 실행
	HINSTANCE hInstance = ShellExecute(NULL, _T("open"), filePath, NULL, NULL, SW_SHOWNORMAL);

	// 선택된 파일 가져오기
	POSITION pos = m_list_image.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		// 선택된 파일이 없음
		return;
	}
	CString m_strImageFolderPath = _T("신고 목록 경로");
	int nSelectedIndex = m_list_image.GetNextSelectedItem(pos);
	CString strSelectedFile = m_list_image.GetItemText(nSelectedIndex, 0);
	CString strSrcFilePath = m_strImageFolderPath + _T("\\") + strSelectedFile;

	// 선택한 파일이 JPG 확장자인지 확인
	CString strExtension = PathFindExtension(strSelectedFile);
	if (strExtension.CompareNoCase(_T(".jpg")) != 0) {
		AfxMessageBox(_T("JPG 파일이 아닙니다."));
		return;
	}
	
	std::this_thread::sleep_for(std::chrono::seconds(2)); //전송 사진이 먼저 삭제가 이루어져 지연 사용

	// 파일 삭제
	if (DeleteFile(strSrcFilePath) == FALSE)
	{
		// 삭제 실패
		DWORD error = GetLastError();
		CString errorMsg;
		errorMsg.Format(_T("신고 접수가 실패했습니다. 오류 코드: %d"), error);
		AfxMessageBox(errorMsg);
		return;
	}
	// 삭제 성공
	AfxMessageBox(_T("신고가 접수되었습니다."));
	
	// 리스트 컨트롤 다시 불러오기
	m_list_image.DeleteAllItems();
	LoadImages();

	if (m_isVideoPlaying) //영상 실행 유무
		OnTimer(m_timerId2);
}

void Ccapstone5Dlg::OnBnClickedButtonDelete()
{
	// 선택된 파일 가져오기
	POSITION pos = m_list_image.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		// 선택된 파일이 없음
		return;
	}
	CString m_strImageFolderPath = _T("신고 목록 경로");
	int nSelectedIndex = m_list_image.GetNextSelectedItem(pos);
	CString strSelectedFile = m_list_image.GetItemText(nSelectedIndex, 0);
	CString strSrcFilePath = m_strImageFolderPath + _T("\\") + strSelectedFile;

	// 선택한 파일이 JPG 확장자인지 확인
	CString strExtension = PathFindExtension(strSelectedFile);
	if (strExtension.CompareNoCase(_T(".jpg")) != 0) {
		AfxMessageBox(_T("JPG 파일이 아닙니다."));
		return;
	}

	// 파일 삭제
	if (DeleteFile(strSrcFilePath) == FALSE)
	{
		// 삭제 실패
		DWORD error = GetLastError();
		CString errorMsg;
		errorMsg.Format(_T("파일을 삭제하는데 실패했습니다. 오류 코드: %d"), error);
		AfxMessageBox(errorMsg);
		return;
	}
	// 삭제 성공
	AfxMessageBox(_T("신고 접수가 취소되었습니다."));
	m_list_image.DeleteAllItems();
	LoadImages();
	
	if (m_isVideoPlaying) //영상 실행 유무
		OnTimer(m_timerId1);
}

void Ccapstone5Dlg::OnBnClickedCancel()
{
	m_list_image.DeleteAllItems();
	LoadImages();

	CDialog::OnOK();
}

void Ccapstone5Dlg::LoadImages()
{
	// 이미지 폴더 경로 설정
	CString m_strImageFolderPath = _T("신고 목록 경로");

	// 폴더 내 모든 파일 목록 가져오기
	CFileFind finder;
	BOOL bWorking = finder.FindFile(m_strImageFolderPath + _T("\\*.*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// 파일이 폴더인 경우 건너뛰기
		if (finder.IsDirectory())
			continue;

		// 파일 이름 및 경로 가져오기
		CString strFileName = finder.GetFileName();
		CString strFilePath = finder.GetFilePath();

		// 파일 확장자가 JPG인 경우 리스트 컨트롤에 추가
		CString strExtension = PathFindExtension(strFileName);
		if (strExtension.CompareNoCase(_T(".jpg")) == 0) 
		{
			int nItem = m_list_image.InsertItem(m_list_image.GetItemCount(), strFileName);
			m_list_image.SetItemText(nItem, 1, strFilePath);
		}
	}
}