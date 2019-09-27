#include "connectargsdlg.h"

IMPLEMENT_CLASS(ConnectArgsDialog,wxDialog)

ConnectArgsDialog::ConnectArgsDialog()
{
   Init();
}

ConnectArgsDialog::ConnectArgsDialog(wxWindow *parent,wxWindowID id,const wxString &caption,const wxPoint &pos,const wxSize &size,long style)
{
   Create(parent,id,caption,pos,size,style);
   Init();
}

inline void ConnectArgsDialog::Init()
{
}

bool ConnectArgsDialog::Create(wxWindow *parent,wxWindowID id,const wxString &caption,const wxPoint &pos,const wxSize &size,long style)
{
   SetExtraStyle(wxWS_EX_BLOCK_EVENTS | wxDIALOG_EX_CONTEXTHELP);

   if(!wxDialog::Create(parent,id,caption,pos,size,style)){
      return false;
   }

   CreateControls();

   GetSizer()->Fit(this);
   GetSizer()->SetSizeHints(this);

   Center();

   return true;
}

void ConnectArgsDialog::CreateControls()
{
   wxBoxSizer *top = new wxBoxSizer(wxVERTICAL);
   this->SetSizer(top);

   wxBoxSizer *box = new wxBoxSizer(wxHORIZONTAL);
   top->Add(box,0,wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,5);

   std::vector<std::string> vector_paths = enumserial.EnumSerialPort();
   wxArrayString paths;
   for(size_t i = 0;i < vector_paths.size();++i){
      paths.Add(vector_paths[i]);
   }

   device_path = new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,paths);
   box->Add(device_path,0,wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,5);

   wxArrayString speed;
   speed.Add(wxT("300"));
   speed.Add(wxT("1200"));
   speed.Add(wxT("2400"));
   speed.Add(wxT("4800"));
   speed.Add(wxT("9600"));
   speed.Add(wxT("19200"));
   speed.Add(wxT("38400"));
   speed.Add(wxT("57600"));
   speed.Add(wxT("115200"));

   baud_rate = new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,speed);
   box->Add(baud_rate,0,wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,5);

   wxBoxSizer *ResetOkCancelBox = new wxBoxSizer(wxHORIZONTAL);
   top->Add(ResetOkCancelBox,0,wxALIGN_CENTER_HORIZONTAL | wxALL,5);

   wxButton *ok = new wxButton(this,wxID_OK,wxT("&Ok"),wxDefaultPosition,wxDefaultSize,0);
   ResetOkCancelBox->Add(ok,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

   wxButton *cancel = new wxButton(this,wxID_CANCEL,wxT("&Cancel"),wxDefaultPosition,wxDefaultSize,0);
   ResetOkCancelBox->Add(cancel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);
}

bool ConnectArgsDialog::TransferDataToWindow()
{
   return true;
}

bool ConnectArgsDialog::TransferDataFromWindow()
{
   return true;
}
