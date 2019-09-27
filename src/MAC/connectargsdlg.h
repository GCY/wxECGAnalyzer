#ifndef __CONNECT_ARGS_DIALOG__
#define __CONNECT_ARGS_DIALOG__
#include <wx/wx.h>

#include "enumserial.h"

class ConnectArgsDialog:public wxDialog
{
   public:
      ConnectArgsDialog();

      ConnectArgsDialog(wxWindow *parent,
	    wxWindowID id = wxID_ANY,
	    const wxString &caption = wxT("Connect Args"),
	    const wxPoint &pos = wxDefaultPosition,
	    const wxSize &size = wxDefaultSize,
	    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

      bool Create(wxWindow *parent,
	    wxWindowID id = wxID_ANY,
	    const wxString &caption = wxT("Connect Args"),
	    const wxPoint &pos = wxDefaultPosition,
	    const wxSize &size = wxDefaultSize,
	    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

      void Init();
      void CreateControls();

      bool TransferDataToWindow();
      bool TransferDataFromWindow();

#ifdef _WIN_ 
	  wxString GetDevicePath() { return device_path->GetString(device_path->GetCurrentSelection()); }
#elif _MAC_
      wxString GetDevicePath(){return enumserial.GetRoot() + device_path->GetString(device_path->GetCurrentSelection());}
#endif
      wxString GetBaudRate(){return baud_rate->GetString(baud_rate->GetCurrentSelection());}

   private:

      EnumSerial enumserial;

      wxChoice *device_path;
      wxChoice *baud_rate;

      DECLARE_CLASS(ConnectArgsDialog)
};

#endif
