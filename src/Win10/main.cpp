#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include <wx/valnum.h>
#include <wx/thread.h>

#include "mathplot.h"
#include "connectargsdlg.h"
#include "serialport.h"

#include "FIR.h"
#include "FFT.h"

#include "adaptive_algorithm.h"
#include "HC_Chen_detect.h"
#include "So_Chen_detect.h"
#include "Pan_Tompkins_detect.h"

#include "HRV.h"

#include "define.h"

#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime> 
#include <iostream>
#include <chrono> 
#include <map>
#include <vector>

enum{
   ID_EXIT = 200,
   ID_OPEN,
   ID_SAVE,
   ID_RECORD,
   ID_VCP,
   ID_CLEAR_ALL_PLOT,
   ID_CONNECT_DEVICE,
   ID_SAVE_SNAPSHOT
};

enum{
   EVT_SERIAL_PORT_READ = 625,
   EVT_REFRESH_PLOT
};

class Thread;
/*
class Plot:public mpWindow
{
   public:

      Plot(wxWindow * 	parent,
	    wxWindowID 	id,
	    const wxPoint & 	pos = wxDefaultPosition,
	    const wxSize & 	size = wxDefaultSize,
	    long 	flags = 0):mpWindow(parent,id,pos,size,flags){
	 x_position = 0;
	 y_position = 0;
      }

      void OnMouseRightDown(wxMouseEvent &event){
	 int m_scrX,m_scrY;
	 GetClientSize(&m_scrX, &m_scrY);
	 if(event.GetX() > 100 && event.GetX() < m_scrX){
	    if(event.GetY() > 0 && event.GetY() < (m_scrY - 50)){
	       x_position = event.GetX();
	       y_position = event.GetY();
	    }
	 }
      }

      void OnMouseRightUp(wxMouseEvent &event){
      }

      uint32_t GetScrX(){
	 return m_scrX;
      }
      uint32_t GetScrY(){
	 return m_scrY;
      }
      uint32_t GetPositionX(){
	 return x_position;
      }
      uint32_t GetPositionY(){
	 return y_position;
      }

   private:
      uint32_t x_position;
      uint32_t y_position;

      DECLARE_EVENT_TABLE();
};

   BEGIN_EVENT_TABLE(Plot,mpWindow)
   EVT_RIGHT_DOWN(Plot::OnMouseRightDown)
   EVT_RIGHT_UP(Plot::OnMouseRightUp)
END_EVENT_TABLE()
*/

   class App:public wxApp
{
   public:
      bool OnInit();
};

class Frame:public wxFrame
{
   public:
      Frame(const wxString&);
      ~Frame();

      void CreateUI();
      void InitPlot();

      void OnProcess(wxCommandEvent&);

      void CalculateFrequency(std::vector<double>&);
      void SnapshotQRSComplex(int,double,SignalPoint);

      void OnRecord(wxCommandEvent&);
      void OnClearAllPlot(wxCommandEvent&);
      void OnOpen(wxCommandEvent&);
      void OnSave(wxCommandEvent&);
      void OnKeyDown(wxKeyEvent& event);
      void OnConnectDevice(wxCommandEvent&);
      void OnExit(wxCommandEvent&);
      void OnSaveSnapshot(wxCommandEvent&);

      void ClearAllData();

      void OnFit(wxCommandEvent&);

      void OnThreadEvent(wxThreadEvent &event);

   private:

      SignalPoint AdaptiveThresholdAlgorithm(double);
      SignalPoint HC_Chen_Algorithm(double);
      SignalPoint So_Chen_Algorithm(double);
      SignalPoint Pan_Tompkins_Algorithm(double);

      wxMenu *vcp;
      wxThread *thread;

      SerialPort serial;     
      bool is_open;

      bool run_flag;

      mpWindow *original_plot;
      mpWindow *processing_plot;  

      mpFXYVector* original_layer;
      std::vector<double> original_layer_x, original_layer_y;

      mpFXYVector* processing_layer;
      std::vector<double> processing_layer_x, processing_layer_y;
      mpFXYVector* r_layer;
      std::vector<double> r_layer_x, r_layer_y;
      mpFXYVector* qs_layer;
      std::vector<double> qs_layer_x, qs_layer_y;       
      static const wxString R_str;
      static const wxString QS_str;

      mpWindow *FFT_plot;
      mpFXYVector *FFT_layer;

	  mpWindow *snapshot_qrs_plot;
      mpFXYVector *snapshot_qrs_layer;
      std::vector<double> snapshot_layer_x, snapshot_layer_y;
      std::vector<double> snapshot_window_x, snapshot_window_y;
      uint32_t snapshot_counter = 0;
      bool snapshot_flag = false;
      std::vector<double> r_index;
      std::vector<double> r_value;      

      std::chrono::steady_clock::time_point start_time;
      int time_to_index;      
      std::chrono::steady_clock::time_point record_start_time;
      bool record_flag;
      std::fstream record;
      std::chrono::steady_clock::time_point fft_last_time;

      std::chrono::steady_clock::time_point beat_out_time;
      std::fstream beat_file;
      uint32_t beat_out_count;

      wxRadioBox *qrs_algorithm_item;
      wxCheckBox *filter_select;

      wxRadioBox *qrs_label_item;     

      std::vector<std::string> serial_pool;

      uint32_t sampling_rate;
      std::chrono::steady_clock::time_point sampling_time;

      DECLARE_EVENT_TABLE();
};

class Thread:public wxThread
{
   public:
      Thread(Frame*,wxEvtHandler*);

      void* Entry();

   private:
      std::chrono::steady_clock::time_point refresh_last;
      std::chrono::steady_clock::time_point read_last;
      Frame *frame;
      wxEvtHandler *handler;
};

   IMPLEMENT_APP(App)
   DECLARE_APP(App)
   BEGIN_EVENT_TABLE(Frame,wxFrame)
   EVT_MENU(ID_EXIT,Frame::OnExit)
   EVT_MENU(ID_OPEN,Frame::OnOpen)
   EVT_MENU(ID_SAVE,Frame::OnSave)
   EVT_MENU(ID_CLEAR_ALL_PLOT,Frame::OnClearAllPlot)
   EVT_MENU(ID_RECORD,Frame::OnRecord)
   EVT_MENU(ID_CONNECT_DEVICE,Frame::OnConnectDevice)
   EVT_MENU(mpID_FIT, Frame::OnFit)
EVT_BUTTON(ID_SAVE_SNAPSHOT,Frame::OnSaveSnapshot)
   //EVT_IDLE(Frame::OnIdle)
   EVT_THREAD(wxID_ANY, Frame::OnThreadEvent)
END_EVENT_TABLE()

   const wxString Frame::R_str = wxT("R");
   const wxString Frame::QS_str = wxT("QS");

bool App::OnInit()
{
   Frame *frame = new Frame(wxT("wxECGAnalyzer"));

   frame->Show(true);

   return true;
}

Frame::Frame(const wxString &title):wxFrame(NULL,wxID_ANY,title,wxDefaultPosition,wxSize(1250,700),wxMINIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxSYSTEM_MENU)
{

   wxInitAllImageHandlers();

   run_flag = true;
   record_flag = false;

   CreateUI();
   InitPlot();

   Bind(wxEVT_CHAR_HOOK,&Frame::OnKeyDown,this);
   
   sampling_time = std::chrono::steady_clock::now();
   sampling_rate = 0;

   start_time = std::chrono::steady_clock::now();
   time_to_index = 0;

   fft_last_time = std::chrono::steady_clock::now();

   beat_out_count = 0;

   thread = new Thread(this,this);
   thread->Create();
   thread->Run();
}

Frame::~Frame()
{
   if(record.is_open()){
      record.close();
   }
   if(thread){
      thread->Delete();
      thread = NULL;
   }
}

void Frame::CreateUI()
{
   wxMenu *file = new wxMenu;
   file->Append(ID_OPEN,wxT("O&pen\tAlt-o"),wxT("Open"));
   file->AppendSeparator();
   file->Append(ID_SAVE,wxT("S&ave\tAlt-s"),wxT("Save"));
   file->Append(ID_RECORD,wxT("R&cord\tAlt-r"),wxT("Record"));
   file->AppendSeparator();
   file->Append(ID_EXIT,wxT("E&xit\tAlt-e"),wxT("Exit"));

   vcp = new wxMenu();
   wxMenu *tools = new wxMenu;

   tools->Append(ID_CONNECT_DEVICE,wxT("V&CP\tAlt-v"),wxT("Virtual COM Port"));

   wxMenu *view = new wxMenu;
   view->Append(mpID_FIT,wxT("F&it\tAlt-f"),wxT("Fit"));
   view->Append(ID_CLEAR_ALL_PLOT,wxT("C&lear Plot\tAlt-c"),wxT("Clear All Plot"));

   wxMenuBar *bar = new wxMenuBar;

   bar->Append(file,wxT("File"));
   bar->Append(tools,wxT("Tools"));
   bar->Append(view,wxT("View"));
   SetMenuBar(bar);

   /* Group1 */
   original_plot = new mpWindow( this, -1, wxPoint(15,10), wxSize(1020,300), wxBORDER_SUNKEN );
   original_plot->EnableDoubleBuffer(true);
   processing_plot = new mpWindow( this, -1, wxPoint(15,330), wxSize(1020,300), wxBORDER_SUNKEN );
   processing_plot->EnableDoubleBuffer(true);

   wxBoxSizer *top = new wxBoxSizer(wxHORIZONTAL);
   this->SetSizer(top);

   wxBoxSizer *group1 = new wxBoxSizer(wxVERTICAL);
   top->Add(group1,1,wxEXPAND);
   group1->Add(original_plot,1,wxEXPAND);
   group1->Add(processing_plot,1,wxEXPAND);

   wxBoxSizer *algorithm_box = new wxBoxSizer(wxVERTICAL);
   group1->Add(algorithm_box,0,wxALIGN_CENTER_HORIZONTAL | wxALL,5);
   wxArrayString qrs_algorithm_item_string;
   qrs_algorithm_item_string.Add(wxT("AdaptiveThresholdAlgorithm"));
   qrs_algorithm_item_string.Add(wxT("HC_Chen"));
   qrs_algorithm_item_string.Add(wxT("Enhanced So-Chen"));
   qrs_algorithm_item_string.Add(wxT("Pan-Tompkins"));   
   qrs_algorithm_item = new wxRadioBox(this,wxID_ANY,wxT("QRS Complex Detect Algorithm"),
	 wxDefaultPosition,wxDefaultSize,qrs_algorithm_item_string,3,wxRA_SPECIFY_COLS);
   algorithm_box->Add(qrs_algorithm_item,0,wxALIGN_CENTER_HORIZONTAL | wxALL,3);   

   filter_select = new wxCheckBox(this, wxID_ANY, "FIR-Filter");
   filter_select->SetValue(true);
   algorithm_box->Add(filter_select,0,wxALIGN_CENTER_HORIZONTAL | wxALL,3);

   /* Group2 */
   FFT_plot = new mpWindow( this, -1, wxPoint(15,10), wxSize(1020,300), wxBORDER_SUNKEN );
   FFT_plot->EnableDoubleBuffer(true);
   snapshot_qrs_plot = new mpWindow( this, -1, wxPoint(15,330), wxSize(1020,300), wxBORDER_SUNKEN );
   snapshot_qrs_plot->EnableDoubleBuffer(true);

   wxBoxSizer *group2 = new wxBoxSizer(wxVERTICAL);
   top->Add(group2,1,wxEXPAND);
   group2->Add(FFT_plot,1,wxEXPAND);   
   group2->Add(snapshot_qrs_plot,1,wxEXPAND);

   wxBoxSizer *label_box = new wxBoxSizer(wxVERTICAL);
   group2->Add(label_box,0,wxALIGN_CENTER_HORIZONTAL | wxALL);
   wxArrayString qrs_label_item_string;
   qrs_label_item_string.Add(wxT("NOT-QRS"));		//not-QRS
   qrs_label_item_string.Add(wxT("Normal"));		//Normal beat
   qrs_label_item_string.Add(wxT("LBBB"));		//Left bundle branch block beat
   qrs_label_item_string.Add(wxT("RBBB"));		//Right bundle branch block beat
   qrs_label_item_string.Add(wxT("ABERR"));		//Aberrated atrial premature beat
   qrs_label_item_string.Add(wxT("PVC"));		//Premature ventricular contraction
   qrs_label_item_string.Add(wxT("FUSION"));		//Fusion of ventricular and normal beat
   qrs_label_item_string.Add(wxT("NPC"));		//Nodal (junctional) premature beat
   qrs_label_item_string.Add(wxT("APC"));		//Atrial premature beat
   qrs_label_item_string.Add(wxT("SVPB"));		//Premature or ectopic supraventricular beat
   qrs_label_item_string.Add(wxT("VESC"));		//Ventricular escape beat
   qrs_label_item_string.Add(wxT("NESC"));		//Nodal (junctional) escape beat
   qrs_label_item_string.Add(wxT("PACE"));		//Paced beat
   qrs_label_item_string.Add(wxT("UNKNOWN"));		//Unclassifiable beat
   qrs_label_item_string.Add(wxT("NOISE"));		//Signal quality change
   qrs_label_item_string.Add(wxT("ARFCT"));		//Isolated QRS-like artifact
   qrs_label_item = new wxRadioBox(this,wxID_ANY,wxT("QRS Complex Label"),
	 wxDefaultPosition,wxDefaultSize,qrs_label_item_string,6,wxRA_SPECIFY_COLS);
   label_box->Add(qrs_label_item,0,wxALIGN_CENTER_HORIZONTAL | wxALL);   

   wxBoxSizer *button_box = new wxBoxSizer(wxHORIZONTAL);
   group2->Add(button_box,0,wxALIGN_CENTER_HORIZONTAL | wxALL);
   wxButton *snapshot_button = new wxButton(this,ID_SAVE_SNAPSHOT,wxT("&Snapshot"),wxDefaultPosition,wxSize(100,15));
#ifdef _WIN_
   button_box->Add(snapshot_button,0,wxALIGN_CENTER_VERTICAL | wxALL);
#elif _MAC_
   button_box->Add(snapshot_button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
#endif
   CreateStatusBar(1);
   SetStatusText(wxT("wxECGAnalyzer"));   
}

void Frame::InitPlot()
{
   original_layer = new mpFXYVector(wxT("RAW Signal"),mpALIGN_NE);
   original_layer->ShowName(false);
   original_layer->SetContinuity(true);
#ifdef _WIN_
   wxPen vectorpen(*wxRED, 2, wxPENSTYLE_SOLID);
#elif _MAC_
   wxPen vectorpen(*wxRED, 2, wxSOLID);
#endif
   original_layer->SetPen(vectorpen);
   original_layer->SetDrawOutsideMargins(false);

   wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
   mpScaleX* xaxis = new mpScaleX(wxT("Time（index）"), mpALIGN_BOTTOM, true, mpX_NORMAL);
   mpScaleY* yaxis = new mpScaleY(wxT("Amplitude（12bit）"), mpALIGN_LEFT, true);
   xaxis->SetFont(graphFont);
   yaxis->SetFont(graphFont);
   xaxis->SetDrawOutsideMargins(false);
   yaxis->SetDrawOutsideMargins(false);
   original_plot->SetMargins(30, 30, 50, 100);
   original_plot->AddLayer(     xaxis );
   original_plot->AddLayer(     yaxis );
   original_plot->AddLayer(     original_layer );

   mpInfoLegend* leg;
   original_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
   leg->SetVisible(true);

   original_plot->EnableDoubleBuffer(true);
   original_plot->SetMPScrollbars(false);
   original_plot->Fit();

   processing_layer = new mpFXYVector(wxT("Processing Signal"),mpALIGN_NE);
   processing_layer->ShowName(false);
   processing_layer->SetContinuity(true);
#ifdef _WIN_
   wxPen vectorpen1(*wxCYAN, 2, wxPENSTYLE_SOLID);
#elif _MAC_
   wxPen vectorpen1(*wxCYAN, 2, wxSOLID);
#endif
   processing_layer->SetPen(vectorpen1);
   processing_layer->SetDrawOutsideMargins(false);

   xaxis = new mpScaleX(wxT("Time（index）"), mpALIGN_BOTTOM, true, mpX_NORMAL);
   yaxis = new mpScaleY(wxT("Amplitude（12bit）"), mpALIGN_LEFT, true);
   xaxis->SetFont(graphFont);
   yaxis->SetFont(graphFont);
   xaxis->SetDrawOutsideMargins(false);
   yaxis->SetDrawOutsideMargins(false);
   processing_plot->SetMargins(30, 30, 50, 100);
   processing_plot->AddLayer(     xaxis );
   processing_plot->AddLayer(     yaxis );
   processing_plot->AddLayer(     processing_layer );

   processing_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
   leg->SetVisible(true);

   processing_plot->EnableDoubleBuffer(true);
   processing_plot->SetMPScrollbars(false);
   processing_plot->Fit();

   r_layer = new mpFXYVector(R_str,mpALIGN_NE);
   r_layer->ShowName(false);
   r_layer->SetContinuity(false);
#ifdef _WIN_
   wxPen vectorpen1_2(*wxRED, 7, wxPENSTYLE_DOT_DASH);
#elif _MAC_
   wxPen vectorpen1_2(*wxRED, 7, wxUSER_DASH);
#endif
   r_layer->SetPen(vectorpen1_2);
   r_layer->SetDrawOutsideMargins(false);   
   processing_plot->AddLayer(     r_layer );

   qs_layer = new mpFXYVector(QS_str,mpALIGN_NE);
   qs_layer->ShowName(false);
   qs_layer->SetContinuity(false);
#ifdef _WIN_
   wxPen vectorpen1_3(*wxGREEN, 7, wxPENSTYLE_DOT_DASH);
#elif _MAC_
   wxPen vectorpen1_3(*wxGREEN, 7, wxUSER_DASH);
#endif
   qs_layer->SetPen(vectorpen1_3);
   qs_layer->SetDrawOutsideMargins(false);   
   processing_plot->AddLayer(     qs_layer );  

   FFT_layer = new mpFXYVector(wxT("FFT"));

   xaxis = new mpScaleX(wxT("Frequency（Hz）"), mpALIGN_BOTTOM, true, mpX_NORMAL);
   yaxis = new mpScaleY(wxT("Amplitude"), mpALIGN_LEFT, true);
   xaxis->SetFont(graphFont);
   yaxis->SetFont(graphFont);
   xaxis->SetDrawOutsideMargins(false);
   yaxis->SetDrawOutsideMargins(false);
   FFT_plot->SetMargins(30, 30, 50, 100);
#ifdef _WIN_
   wxPen vectorpen3(*wxYELLOW, 2, wxPENSTYLE_SOLID);
#elif _MAC_
   wxPen vectorpen3(*wxYELLOW, 2, wxSOLID);
#endif
   FFT_layer->SetPen(vectorpen3);   
   FFT_plot->AddLayer(     xaxis );
   FFT_plot->AddLayer(     yaxis );
   FFT_plot->AddLayer(     FFT_layer );

   FFT_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
   leg->SetVisible(true);    

   FFT_plot->EnableDoubleBuffer(true);
   FFT_plot->SetMPScrollbars(false);
   FFT_plot->Fit();        

   snapshot_qrs_layer = new mpFXYVector(wxT("Snapshot"));
   xaxis = new mpScaleX(wxT("Time（index）"), mpALIGN_BOTTOM, true, mpX_NORMAL);
   yaxis = new mpScaleY(wxT("Amplitude（12bit）"), mpALIGN_LEFT, true);
   xaxis->SetFont(graphFont);
   yaxis->SetFont(graphFont);
   xaxis->SetDrawOutsideMargins(false);
   yaxis->SetDrawOutsideMargins(false);
   snapshot_qrs_plot->SetMargins(30, 30, 50, 100);
   snapshot_qrs_plot->AddLayer(     xaxis );
   snapshot_qrs_plot->AddLayer(     yaxis );   
   snapshot_qrs_plot->AddLayer(     snapshot_qrs_layer );

   snapshot_qrs_plot->AddLayer(new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
   leg->SetVisible(true); 

   snapshot_qrs_plot->EnableDoubleBuffer(true);
   snapshot_qrs_plot->SetMPScrollbars(false);
   snapshot_qrs_plot->Fit();      
}

void Frame::OnFit( wxCommandEvent &WXUNUSED(event) )
{
   original_plot->Fit();
   processing_plot->Fit();
   snapshot_qrs_plot->Fit();
   FFT_plot->Fit();
}

void Frame::OnClearAllPlot(wxCommandEvent &event)
{
   ClearAllData();
}

void Frame::ClearAllData()
{
   original_plot->DelAllLayers(true);
   processing_plot->DelAllLayers(true);
   snapshot_qrs_plot->DelAllLayers(true);
   FFT_plot->DelAllLayers(true);   

   original_layer_x.clear();
   original_layer_y.clear();
   processing_layer_x.clear();
   processing_layer_y.clear();
}

void Frame::OnOpen(wxCommandEvent &event)
{
   wxFileDialog 
      file_dialog(this, wxT("Open RAW Data file"), "", "",
	    "RAW Data files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
   if(file_dialog.ShowModal() == wxID_CANCEL){
      return;
   }

   event.Skip();
}

void Frame::OnSave(wxCommandEvent &event)
{
   wxFileDialog file_dialog(this, _(""), wxT(""), wxT(""), wxT(""), wxFD_SAVE);
   if(file_dialog.ShowModal() == wxID_OK) {
      wxSize size(1020,300);
      original_plot->SaveScreenshot(file_dialog.GetPath() + wxT("_raw.png"), wxBITMAP_TYPE_PNG, size, false);
      processing_plot->SaveScreenshot(file_dialog.GetPath() + wxT("_filter.png"), wxBITMAP_TYPE_PNG, size, false);
      FFT_plot->SaveScreenshot(file_dialog.GetPath() + wxT("_FFT.png"), wxBITMAP_TYPE_PNG, size, false);
      snapshot_qrs_plot->SaveScreenshot(file_dialog.GetPath() + wxT("_QRS.png"), wxBITMAP_TYPE_PNG, size, false);
   }
   else{
      return;
   }   

   event.Skip();
}

void Frame::OnRecord(wxCommandEvent &event)
{

   wxFileDialog file_dialog(this, _("CSV"), wxT(""), wxT("data"), wxT("CSV (*.csv)|.csv"), wxFD_SAVE);
   if(file_dialog.ShowModal() == wxID_OK) {
      record.open(file_dialog.GetPath().mb_str(),std::fstream::out);
      record_flag = true;
      record_start_time = std::chrono::steady_clock::now();
   }
   else{
      return;
   }

   event.Skip();
}

void Frame::OnSaveSnapshot(wxCommandEvent &event)
{
   bool temp_run_flag = run_flag;
   run_flag &= false;   
   wxFileDialog fileDialog(this, _("CSV"), wxT(""), wxT("data"), wxT("CSV (*.csv)|.csv"), wxFD_SAVE);
   if(fileDialog.ShowModal() == wxID_OK) {
      wxFileName namePath(fileDialog.GetPath());
      std::fstream snapshot_file;
      snapshot_file.open(fileDialog.GetPath().mb_str(),std::fstream::out);

      snapshot_file << "Sampling Rate" << "," << SAMPLING_RATE << std::endl;
      snapshot_file << "QRS_Index" << "," << r_index[0] << std::endl;
      snapshot_file << "QRS_Value" << "," << r_value[0] << std::endl;
      snapshot_file << "Label" << "," << qrs_label_item->GetSelection() << std::endl;

      for(size_t i = 0;i < snapshot_layer_x.size();++i){
	 snapshot_file << snapshot_layer_x[i] << "," << snapshot_layer_y[i] << std::endl;
      }

      snapshot_file.close();
   }
   run_flag = temp_run_flag;
}

void Frame::OnConnectDevice(wxCommandEvent &event)
{
   ConnectArgsDialog dlg(this,wxID_ANY,wxT("Connect"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE);

   if(dlg.ShowModal() == wxID_OK){
#ifdef _WIN_
      is_open = serial.Open(dlg.GetDevicePath().wc_str());
#elif _MAC_
      is_open = serial.Open(dlg.GetDevicePath().c_str());
#endif
      serial.SetBaudRate(wxAtoi(dlg.GetBaudRate()));
      serial.SetParity(8,1,'N');

      run_flag = true;

      if(is_open){
	 unsigned char gid[4] = "ACK";
	 serial.Write(gid,4);

	 unsigned char sms[4] = "GOT";
	 serial.Write(sms,4);  
      }
      else{
	 wxMessageBox(wxT("Serial Port Error!"),wxT("Error!"));
      }

   }
}

void Frame::OnKeyDown(wxKeyEvent& event)
{
   if(event.GetKeyCode() == 32){
      run_flag ^= true;
   }
   else if(event.GetKeyCode() == 'B' || event.GetKeyCode() == 'b'){
      if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - beat_out_time).count() < (std::chrono::steady_clock::period::den * 60)){
	 return;
      }
      beat_out_count++;
      beat_out_time = std::chrono::steady_clock::now();
      char str[255];
      sprintf(str,"beat%d.csv",beat_out_count);
      beat_file.open(str,std::fstream::out);
   }

   event.Skip();
}

void Frame::OnExit(wxCommandEvent &event)
{
   Close();
}

SignalPoint Frame::AdaptiveThresholdAlgorithm(double value)
{
   processing_layer_x.push_back( time_to_index );
   if(filter_select->IsChecked()){value = FIR_filter(value);}
   processing_layer_y.push_back(value);

   static const float CV_LIMIT = 50.0f;
   static const float THRESHOLD_FACTOR = 3.0f;
   double mean = CalculateMean(value);
   double rms = CalculateRootMeanSquare(value);
   double cv = CalculateCoefficientOfVariation(value);
   double threshold;
   if(cv > CV_LIMIT){
      threshold = rms;
   }
   else{
      threshold = rms * (cv/100.0f) * THRESHOLD_FACTOR;
   }		  
   bool is_peak;
   SignalPoint result;
   result = PeakDetect(value,time_to_index,threshold,&is_peak);
   if(result.index != -1){
      if(is_peak){
	 AddBeatArray(result.index);
	 r_layer_x.push_back(result.index);
	 r_layer_y.push_back(result.value);

	 return result;
      }
      else{
	 qs_layer_x.push_back(result.index);
	 qs_layer_y.push_back(result.value);			
      }
   }

   result.index = -1;
   return result;
}

SignalPoint Frame::HC_Chen_Algorithm(double value)
{
   processing_layer_x.push_back( time_to_index );
   double result = value;
   if(filter_select->IsChecked()){result = FIR_filter(value);}
   processing_layer_y.push_back(result);
   bool is_peak = HC_Chen_detect(result);
   if(is_peak){
      AddBeatArray(time_to_index);
      r_layer_x.push_back(time_to_index);
      r_layer_y.push_back(result);

      SignalPoint peak;
      peak.index = time_to_index;
      peak.value = result;
      return peak;
   }

   SignalPoint peak;
   peak.index = -1;
   return peak;
}

SignalPoint Frame::So_Chen_Algorithm(double value)
{
   processing_layer_x.push_back( time_to_index );
   double result = value;
   if(filter_select->IsChecked()){result = FIR_filter(value);}
   processing_layer_y.push_back(result);
   SignalPoint sp;
   sp.value = result;
   sp.index = time_to_index;
   SignalPoint peak = So_Chen_detect(sp,SAMPLING_RATE * 0.25f,4,16);
   if(peak.index != -1){
      AddBeatArray(time_to_index);
      r_layer_x.push_back(peak.index);
      r_layer_y.push_back(peak.value);

      return peak;
   }

   return peak;
}


SignalPoint Frame::Pan_Tompkins_Algorithm(double value)
{
   processing_layer_x.push_back( time_to_index );
   processing_layer_y.push_back(value);   

   double result = value;
   double bandpass;
   double integral;
   double square;

   // Design by FIR Filter
   if(filter_select->IsChecked()){
      result = FIR_filter(result);
      bandpass = result;
   }
   // for 200Hz sampling rate , band-pass 5Hz~12Hz
   else{
      //result = TwoPoleRecursive(result);
      result = LowPassFilter(result);
      result = HighPassFilter(result);
      bandpass = result;
   }
   result = Derivative(result);
   result = Squar(result);
   square = result;
   result = MovingWindowIntegral(result);
   integral = result;

   SignalPoint peak = ThresholdCalculate(time_to_index,value,bandpass,square,integral);

   if(peak.index != -1){
      AddBeatArray(peak.index);
      r_layer_x.push_back(peak.index);
      r_layer_y.push_back(peak.value);
      return peak;
   }

   return peak;
}

void Frame::CalculateFrequency(std::vector<double> &signal)
{
   double sum = 0;
   for(size_t i = 0;i < signal.size();++i){
      sum += signal[i];
   }
   double DC = (sum / signal.size());   

   int fft_data_length = (int)pow(2.0f, ceil(log((double)signal.size())/log(2.0f)));

   std::vector<double> fft_data((2 * fft_data_length + 1),0.0f);

   for(size_t i = 0;i < signal.size();++i){
      fft_data[2*i+1] = signal[i] - DC;
      fft_data[2*i+2] = 0.0f;
   }

   for(size_t i = processing_layer_y.size();i < fft_data_length;++i){
      fft_data[2*i+1] = 0.0f;
      fft_data[2*i+2] = 0.0f;
   }

   FFT(fft_data,fft_data_length,1);  

   std::vector<double> x_axis,result;
   std::map<double,double> sort_result;
   for(size_t i = 0;i < fft_data_length / 2 + 1;++i){
      result.push_back(sqrt(std::pow(fft_data[2*i+1],2) + std::pow(fft_data[2*i+2],2)));
      x_axis.push_back(i * (SAMPLING_RATE / 2.0f) / (fft_data_length / 2 + 1));

      sort_result.insert(std::make_pair(sqrt(std::pow(fft_data[2*i+1],2) + std::pow(fft_data[2*i+2],2)),i));
   } 


   FFT_plot->DelAllLayers(true);

   FFT_layer = new mpFXYVector(wxT("FFT"));
   FFT_layer->ShowName(false);

   FFT_layer->SetData(x_axis, result);
   FFT_layer->SetContinuity(true);
#ifdef _WIN_
   wxPen vectorpen(*wxYELLOW, 2, wxPENSTYLE_SOLID);
#elif _MAC_
   wxPen vectorpen(*wxYELLOW, 2, wxSOLID);
#endif
   FFT_layer->SetPen(vectorpen);
   FFT_layer->SetDrawOutsideMargins(false);

   wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
   mpScaleX* xaxis = new mpScaleX(wxT("Frequency（Hz）"), mpALIGN_BOTTOM, true, mpX_NORMAL);
   mpScaleY* yaxis = new mpScaleY(wxT("Amplitude"), mpALIGN_LEFT, true);
   xaxis->SetFont(graphFont);
   yaxis->SetFont(graphFont);
   xaxis->SetDrawOutsideMargins(false);
   yaxis->SetDrawOutsideMargins(false);
   FFT_plot->SetMargins(30, 30, 50, 100);
   FFT_plot->AddLayer(     xaxis );
   FFT_plot->AddLayer(     yaxis );
   FFT_plot->AddLayer(     FFT_layer );

   mpInfoLegend* leg;
   FFT_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
   leg->SetVisible(true); 


   std::map<double,double>::reverse_iterator it_max = sort_result.rbegin();
   std::vector<double> index_axis;
   std::vector<double> m_axis;
   index_axis.push_back(it_max->second * (SAMPLING_RATE / 2.0f) / (fft_data_length / 2 + 1));
   m_axis.push_back(it_max->first);

   mpFXYVector* vectorLayer2 = new mpFXYVector(wxString::Format(wxT("%.2fHz"),it_max->second * (SAMPLING_RATE / 2.0f) / (fft_data_length / 2 + 1)));
   vectorLayer2->ShowName(false);   
   vectorLayer2->SetData(index_axis, m_axis);
   vectorLayer2->SetContinuity(false);
#ifdef _WIN_
   wxPen vectorpen2(*wxRED, 7, wxPENSTYLE_DOT_DASH);
#elif _MAC_
   wxPen vectorpen2(*wxRED, 7, wxUSER_DASH);
#endif
   vectorLayer2->SetPen(vectorpen2);
   vectorLayer2->SetDrawOutsideMargins(false);

   FFT_plot->AddLayer(     vectorLayer2 );   

   FFT_plot->EnableDoubleBuffer(true);
   FFT_plot->SetMPScrollbars(false);
   //FFT_plot->Fit();        
}

void Frame::SnapshotQRSComplex(int index,double value,SignalPoint peak)
{
   const float P_to_R = SAMPLING_RATE * 0.1f; //100ms
   const float R_to_T = SAMPLING_RATE * 0.35f; //150ms
   const size_t MAX_POINT = SAMPLING_RATE * 0.8f; //700ms

   static SignalPoint last_peak;

   snapshot_window_x.push_back(index);
   snapshot_window_y.push_back(value);

   if (snapshot_window_x.size() > MAX_POINT && snapshot_window_y.size() > MAX_POINT){
      snapshot_window_x.erase(snapshot_window_x.begin());
      snapshot_window_y.erase(snapshot_window_y.begin());
   }

   if(peak.index != -1){
      snapshot_counter = 0;
      snapshot_flag = true;

      last_peak = peak;
   }

   if(snapshot_flag){
      ++snapshot_counter;

      if(snapshot_counter > (R_to_T)){
	 snapshot_flag = false;
	 snapshot_counter = 0;

	 snapshot_layer_x = snapshot_window_x;
	 snapshot_layer_y = snapshot_window_y;

	 snapshot_qrs_plot->DelAllLayers(true);

	 snapshot_qrs_layer = new mpFXYVector(wxT("Snapshot"));
	 snapshot_qrs_layer->ShowName(false);

	 snapshot_qrs_layer->SetData(snapshot_window_x, snapshot_window_y);
	 snapshot_qrs_layer->SetContinuity(true);
#ifdef _WIN_
	 wxPen vectorpen(*wxBLACK, 2, wxPENSTYLE_SOLID);
#elif _MAC_
	 wxPen vectorpen(*wxBLACK, 2, wxSOLID);
#endif
	 snapshot_qrs_layer->SetPen(vectorpen);  
	 snapshot_qrs_layer->SetDrawOutsideMargins(false);

	 wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	 mpScaleX* xaxis = new mpScaleX(wxT("Time（index）"), mpALIGN_BOTTOM, true, mpX_NORMAL);
	 mpScaleY* yaxis = new mpScaleY(wxT("Amplitude（12bit）"), mpALIGN_LEFT, true);
	 xaxis->SetFont(graphFont);
	 yaxis->SetFont(graphFont);
	 xaxis->SetDrawOutsideMargins(false);
	 yaxis->SetDrawOutsideMargins(false);
	 snapshot_qrs_plot->SetMargins(30, 30, 50, 100);
	 snapshot_qrs_plot->AddLayer(     xaxis );
	 snapshot_qrs_plot->AddLayer(     yaxis );
	 snapshot_qrs_plot->AddLayer(     snapshot_qrs_layer );

	 mpInfoLegend* leg;
	 snapshot_qrs_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
	 leg->SetVisible(true); 

	 r_index.clear();
	 r_value.clear();
	 r_index.push_back(last_peak.index);
	 r_value.push_back(last_peak.value);

	 mpFXYVector* vectorLayer2 = new mpFXYVector(wxT("R"));
	 vectorLayer2->ShowName(false);   
	 vectorLayer2->SetData(r_index, r_value);
	 vectorLayer2->SetContinuity(false);
#ifdef _WIN_
	 wxPen vectorpen2(*wxRED, 7, wxPENSTYLE_DOT_DASH);
#elif _MAC_
	 wxPen vectorpen2(*wxRED, 7, wxUSER_DASH);
#endif
	 vectorLayer2->SetPen(vectorpen2);
	 vectorLayer2->SetDrawOutsideMargins(false);

	 snapshot_qrs_plot->AddLayer(     vectorLayer2 );  

	 snapshot_qrs_plot->EnableDoubleBuffer(true);
	 snapshot_qrs_plot->SetMPScrollbars(false);
	 //snapshot_qrs_plot->Fit();  

      }

   }
}

void Frame::OnThreadEvent(wxThreadEvent &event)
{
   const size_t MAX_POINT = SAMPLING_RATE * 5;

   if(event.GetInt() == EVT_SERIAL_PORT_READ && is_open){

	   unsigned char buffer[30000] = {0};
	  int length = serial.Read(buffer);

      if(length != -1 && run_flag){
	 //std::cout << buffer << std::endl;
		  
	 wxStringTokenizer tokenizer1(buffer,"R");
	 if(tokenizer1.CountTokens() == 0){
	    return ;
	 }

	 tokenizer1.GetNextToken();

	 while(tokenizer1.HasMoreTokens()){
	    wxString split = tokenizer1.GetNextToken();

	    if(record_flag){
	       record << split << std::endl;
	       if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - record_start_time).count() > (std::chrono::steady_clock::period::den * 60)){
		  record_flag = false;
		  record.close();
		  wxMessageBox(wxT("Record 1 minumte data!"),wxT("DONE!"));
	       }
	    }

	    wxStringTokenizer tokenizer2(split,",");
	    if(tokenizer2.CountTokens() < 5){
	       continue;
	    }	    

	    ++sampling_rate;
	    if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - sampling_time).count() > std::chrono::steady_clock::period::den){
	       wxLogDebug(wxT("Sampling_Rate: %d"),sampling_rate);
	       sampling_rate = 0;
	       sampling_time = std::chrono::steady_clock::now();
	    }	    

	    int token_index = 0;
	    while(tokenizer2.HasMoreTokens()){
	       wxString str = tokenizer2.GetNextToken();       
	       if(token_index == 1){
		  original_layer_x.push_back( time_to_index );
		  double value = wxAtoi(str);
		  original_layer_y.push_back(value);
		  if (original_layer_x.size() > MAX_POINT && original_layer_y.size() > MAX_POINT){
		     original_layer_x.erase(original_layer_x.begin());
		     original_layer_y.erase(original_layer_y.begin());
		  } 

		  SignalPoint peak;
		  if(qrs_algorithm_item->GetSelection() == 0){
		     filter_select->Enable(true);
		     peak = AdaptiveThresholdAlgorithm(value);
		     processing_layer->SetName(wxT("AdaptiveThresholdAlgorithm"));
		  }
		  else if(qrs_algorithm_item->GetSelection() == 1){
		     filter_select->Enable(true);
		     peak = HC_Chen_Algorithm(value);
		     processing_layer->SetName(wxT("HC_Chen_Algorithm"));
		  }
		  else if(qrs_algorithm_item->GetSelection() == 2){
		     filter_select->Enable(true);
		     peak = So_Chen_Algorithm(value);
		     processing_layer->SetName(wxT("So_Chen_Algorithm"));
		  }
		  else if(qrs_algorithm_item->GetSelection() == 3){
		     filter_select->Enable(true);
		     peak = Pan_Tompkins_Algorithm(value);
		     processing_layer->SetName(wxT("Pan_Tompkins_Algorithm"));
		  }

		  if (processing_layer_x.size() > MAX_POINT && processing_layer_y.size() > MAX_POINT){
		     processing_layer_x.erase(processing_layer_x.begin());
		     processing_layer_y.erase(processing_layer_y.begin());
		     if(r_layer_x.size()){
			if((time_to_index - r_layer_x[0]) > MAX_POINT){
			   r_layer_x.erase(r_layer_x.begin());
			   r_layer_y.erase(r_layer_y.begin());
			}
		     }
		     if(qs_layer_x.size()){
			if((time_to_index - qs_layer_x[0]) > MAX_POINT){
			   qs_layer_x.erase(qs_layer_x.begin());
			   qs_layer_y.erase(qs_layer_y.begin());
			}
		     }		     
		  }

		  SnapshotQRSComplex(time_to_index,processing_layer_y.at(processing_layer_y.size()-1),peak);

		  if(peak.index != -1){
		     if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - beat_out_time).count() > (std::chrono::steady_clock::period::den * 60)){
			beat_file.close();
		     }
		     else{
			beat_file << PulseRate() << std::endl;
		     }

		  }
		  
		  if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - fft_last_time).count() > std::chrono::steady_clock::period::den){
		     CalculateFrequency(processing_layer_y);
		     fft_last_time = std::chrono::steady_clock::now();
		  }		  
		  
	       }
	       if(token_index == 2){
		  /*
		     processing_layer_x.push_back( time_to_index );
		     processing_layer_y.push_back(wxAtoi(str));
		     if (processing_layer_x.size() > MAX_POINT && processing_layer_y.size() > MAX_POINT){
		     processing_layer_x.erase(processing_layer_x.begin());
		     processing_layer_y.erase(processing_layer_y.begin());
		     } */

		  ++time_to_index;
	       }		       
	       ++token_index;
	    }
	 }

      }  
   }

   if(event.GetInt() == EVT_REFRESH_PLOT && is_open && run_flag){

      if(r_layer_x.size()){
	 if(!processing_plot->IsLayerVisible(R_str)){
	    processing_plot->AddLayer(r_layer);
	 }
	 r_layer->SetData(r_layer_x, r_layer_y);
      }
      else{
	 if(processing_plot->IsLayerVisible(R_str)){
	    processing_plot->DelLayer(r_layer);
	 }
      }
      if(qs_layer_x.size()){
	 if(!processing_plot->IsLayerVisible(QS_str)){
	    processing_plot->AddLayer(qs_layer);
	 }	 
	 qs_layer->SetData(qs_layer_x, qs_layer_y);
      }
      else{
	 if(processing_plot->IsLayerVisible(QS_str)){
	    processing_plot->DelLayer(qs_layer);
	 }	 
      }        
      original_layer->SetData(original_layer_x, original_layer_y);
      original_plot->Fit();
      processing_layer->SetData(processing_layer_x, processing_layer_y);
      processing_plot->Fit();  

	  FFT_plot->Fit();
	  snapshot_qrs_plot->Fit();
	  
      double sdnn = HRV_SDNN();
      double pulse_rate = PulseRate();
      uint32_t nn50 = NN50();
      wxString str;
#ifdef _WIN_
str.Printf(wxT("Heart Rate : %f , HRV-SDNN : %f , HRV-NN50 : %d , HRV-pNN50 : %f%%"), pulse_rate, sdnn, nn50, (float)nn50 / (float)BEAT_DIFF_SIZE*100.0f);
#elif _MAC_
      str.Printf(wxT("Heart Rate : %f , HRV-SDNN : %f , HRV-NN50 : %d , HRV-pNN50 : %f\%"),pulse_rate,sdnn,nn50,(float)nn50/(float)BEAT_DIFF_SIZE*100.0f);
#endif
	  SetStatusText(str);
   }
}

Thread::Thread(Frame *parent,wxEvtHandler *evt):wxThread(wxTHREAD_DETACHED),handler(evt)
{
   frame = parent;
   refresh_last = std::chrono::steady_clock::now();
   read_last = std::chrono::steady_clock::now();
}

void* Thread::Entry()
{
   const int32_t READ_RATE = 100;
   const int32_t FRAME_RATE = 10;

   while(!TestDestroy()){
      if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - read_last).count() > (std::chrono::steady_clock::period::den/READ_RATE)){
	 wxThreadEvent *evt = new wxThreadEvent(wxEVT_THREAD);
	 evt->SetInt(EVT_SERIAL_PORT_READ);
	 handler->QueueEvent(evt);
	 read_last = std::chrono::steady_clock::now();
      }
      if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - refresh_last).count() > (std::chrono::steady_clock::period::den/FRAME_RATE)){
	 wxThreadEvent *evt = new wxThreadEvent(wxEVT_THREAD);
	 evt->SetInt(EVT_REFRESH_PLOT);
	 handler->QueueEvent(evt);
	 refresh_last = std::chrono::steady_clock::now();
      }
   }

   return NULL;
}
