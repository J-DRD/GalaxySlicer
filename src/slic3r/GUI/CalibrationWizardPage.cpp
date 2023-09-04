#include "CalibrationWizardPage.hpp"
#include "I18N.hpp"
#include "Widgets/Label.hpp"
#include "MsgDialog.hpp"

namespace Slic3r { namespace GUI {

wxDEFINE_EVENT(EVT_CALI_ACTION, wxCommandEvent);
wxDEFINE_EVENT(EVT_CALI_TRAY_CHANGED, wxCommandEvent);


CalibrationStyle get_cali_style(MachineObject* obj)
{
    if (!obj) return CalibrationStyle::CALI_STYLE_DEFAULT;

    if (obj->get_printer_series() == PrinterSeries::SERIES_X1)
        return CalibrationStyle::CALI_STYLE_X1;
    else if (obj->get_printer_series() == PrinterSeries::SERIES_P1P)
        return CalibrationStyle::CALI_STYLE_P1P;

    return CalibrationStyle::CALI_STYLE_DEFAULT;
}

wxString get_cali_mode_caption_string(CalibMode mode)
{
    if (mode == CalibMode::Calib_PA_Line)
        return _L("Flow Dynamics Calibration");
    if (mode == CalibMode::Calib_Flow_Rate)
        return _L("Flow Rate Calibration");
    if (mode == CalibMode::Calib_Vol_speed_Tower)
        return _L("Max Volumetric Speed Calibration");
    return "no cali_mode_caption";
}

wxString get_calibration_wiki_page(CalibMode cali_mode)
{
    switch (cali_mode) {
    case CalibMode::Calib_PA_Line:
        return wxString("https://wiki.bambulab.com/en/software/bambu-studio/calibration_pa");
    case CalibMode::Calib_Flow_Rate:
        return wxString("https://wiki.bambulab.com/en/software/bambu-studio/calibration_flow_rate");
    case CalibMode::Calib_Vol_speed_Tower:
        return wxString("https://wiki.bambulab.com/en/software/bambu-studio/calibration_volumetric");
    case CalibMode::Calib_Temp_Tower:
        return wxString("https://wiki.bambulab.com/en/software/bambu-studio/calibration_temperature");
    case CalibMode::Calib_Retraction_tower:
        return wxString("https://wiki.bambulab.com/en/software/bambu-studio/calibration_retraction");
    default:
        return "";
    }
}

CalibrationFilamentMode get_cali_filament_mode(MachineObject* obj, CalibMode mode)
{
    // default 
    if (!obj) return CalibrationFilamentMode::CALI_MODEL_SINGLE;


    if (mode == CalibMode::Calib_PA_Line) {
        if (obj->get_printer_series() == PrinterSeries::SERIES_X1)
            return CalibrationFilamentMode::CALI_MODEL_MULITI;
        else if (obj->get_printer_series() == PrinterSeries::SERIES_P1P)
            return CalibrationFilamentMode::CALI_MODEL_SINGLE;
    }
    else if (mode == CalibMode::Calib_Flow_Rate) {
        if (obj->get_printer_series() == PrinterSeries::SERIES_X1)
            return CalibrationFilamentMode::CALI_MODEL_SINGLE;
        else if (obj->get_printer_series() == PrinterSeries::SERIES_P1P)
            return CalibrationFilamentMode::CALI_MODEL_SINGLE;
    }

    return CalibrationFilamentMode::CALI_MODEL_SINGLE;
}

CalibMode get_obj_calibration_mode(const MachineObject* obj)
{
    CalibrationMethod method;
    int cali_stage;
    return get_obj_calibration_mode(obj, method, cali_stage);
}

CalibMode get_obj_calibration_mode(const MachineObject* obj, int& cali_stage)
{
    CalibrationMethod method;
    return get_obj_calibration_mode(obj, method, cali_stage);
}

CalibMode get_obj_calibration_mode(const MachineObject* obj, CalibrationMethod& method, int& cali_stage)
{
    method = CalibrationMethod::CALI_METHOD_MANUAL;

    if (!obj) return CalibMode::Calib_None;

    if (boost::contains(obj->m_gcode_file, "auto_filament_cali")) {
        method = CalibrationMethod::CALI_METHOD_AUTO;
        return CalibMode::Calib_PA_Line;
    }
    if (boost::contains(obj->m_gcode_file, "user_cali_manual_pa")) {
        method = CalibrationMethod::CALI_METHOD_MANUAL;
        return CalibMode::Calib_PA_Line;
    }
    if (boost::contains(obj->m_gcode_file, "extrusion_cali")) {
        method = CalibrationMethod::CALI_METHOD_MANUAL;
        return CalibMode::Calib_PA_Line;
    }

    if (boost::contains(obj->m_gcode_file, "abs_flowcalib_cali")) {
        method = CalibrationMethod::CALI_METHOD_AUTO;
        return CalibMode::Calib_Flow_Rate;
    }

    if (obj->get_printer_series() == PrinterSeries::SERIES_P1P) {
        if (boost::contains(obj->subtask_name, "auto_filament_cali")) {
            method = CalibrationMethod::CALI_METHOD_AUTO;
            return CalibMode::Calib_PA_Line;
        }
        if (boost::contains(obj->subtask_name, "user_cali_manual_pa")) {
            method = CalibrationMethod::CALI_METHOD_MANUAL;
            return CalibMode::Calib_PA_Line;
        }
        if (boost::contains(obj->subtask_name, "extrusion_cali")) {
            method = CalibrationMethod::CALI_METHOD_MANUAL;
            return CalibMode::Calib_PA_Line;
        }

        if (boost::contains(obj->subtask_name, "abs_flowcalib_cali")) {
            method = CalibrationMethod::CALI_METHOD_AUTO;
            return CalibMode::Calib_Flow_Rate;
        }
    }

    CalibMode cali_mode = CalibUtils::get_calib_mode_by_name(obj->subtask_name, cali_stage);
    if (cali_mode != CalibMode::Calib_None) {
        method = CalibrationMethod::CALI_METHOD_MANUAL;
    }
    return cali_mode;
}


CaliPageButton::CaliPageButton(wxWindow* parent, CaliPageActionType type, wxString text)
    : m_action_type(type),
    Button(parent, text)
{
    StateColor btn_bg_green(std::pair<wxColour, int>(wxColour("#CECECE"), StateColor::Disabled),
        std::pair<wxColour, int>(wxColour("#C7ACCB"), StateColor::Pressed),
        std::pair<wxColour, int>(wxColour("#9C6DA4"), StateColor::Hovered),
        std::pair<wxColour, int>(wxColour("#693A71"), StateColor::Normal));

    StateColor btn_bg_white(std::pair<wxColour, int>(wxColour("#CECECE"), StateColor::Disabled),
        std::pair<wxColour, int>(wxColour("#CECECE"), StateColor::Pressed),
        std::pair<wxColour, int>(wxColour("#EEEEEE"), StateColor::Hovered),
        std::pair<wxColour, int>(wxColour("#FFFFFF"), StateColor::Normal));

    StateColor btn_bd_green(std::pair<wxColour, int>(wxColour("#FFFFFE"), StateColor::Disabled),
        std::pair<wxColour, int>(wxColour("#693A71"), StateColor::Enabled));

    StateColor btn_bd_white(std::pair<wxColour, int>(wxColour("#FFFFFE"), StateColor::Disabled),
        std::pair<wxColour, int>(wxColour("#262E30"), StateColor::Enabled));

    StateColor btn_text_green(std::pair<wxColour, int>(wxColour("#FFFFFE"), StateColor::Disabled),
        std::pair<wxColour, int>(wxColour("#FFFFFE"), StateColor::Enabled));

    StateColor btn_text_white(std::pair<wxColour, int>(wxColour("#FFFFFE"), StateColor::Disabled),
        std::pair<wxColour, int>(wxColour("#262E30"), StateColor::Enabled));

    switch (m_action_type)
    {
    case CaliPageActionType::CALI_ACTION_MANAGE_RESULT:
        this->SetLabel(_L("Manage Result"));
        break;
    case CaliPageActionType::CALI_ACTION_MANUAL_CALI:
        this->SetLabel(_L("Manual Calibration"));
        this->SetToolTip(_L("Result can be read by human eyes."));
        break;
    case CaliPageActionType::CALI_ACTION_AUTO_CALI:
        this->SetLabel(_L("Auto-Calibration"));
        this->SetToolTip(_L("We would use Lidar to read the calibration result"));
        break;
    case CaliPageActionType::CALI_ACTION_START:
        this->SetLabel(_L("Start Calibration"));
        break;
    case CaliPageActionType::CALI_ACTION_PREV:
        this->SetLabel(_L("Prev"));
        break;
    case CaliPageActionType::CALI_ACTION_RECALI:
        this->SetLabel(_L("Recalibration"));
        break;
    case CaliPageActionType::CALI_ACTION_NEXT:
        this->SetLabel(_L("Next"));
        break;
    case CaliPageActionType::CALI_ACTION_CALI_NEXT:
        this->SetLabel(_L("Next"));
        break;
    case CaliPageActionType::CALI_ACTION_CALI:
        this->SetLabel(_L("Calibrate"));
        break;
    case CaliPageActionType::CALI_ACTION_FLOW_CALI_STAGE_2:
        this->SetLabel(_L("Calibrate"));
        break;
    case CaliPageActionType::CALI_ACTION_PA_SAVE:
        this->SetLabel(_L("Finish"));
        break;
    case CaliPageActionType::CALI_ACTION_FLOW_SAVE:
        this->SetLabel(_L("Finish"));
        break;
    case CaliPageActionType::CALI_ACTION_FLOW_COARSE_SAVE:
        this->SetLabel(_L("Finish"));
        break;
    case CaliPageActionType::CALI_ACTION_FLOW_FINE_SAVE:
        this->SetLabel(_L("Finish"));
        break;
    case CaliPageActionType::CALI_ACTION_COMMON_SAVE:
        this->SetLabel(_L("Finish"));
        break;
    default:
        this->SetLabel("Unknown");
        break;
    }

    switch (m_action_type)
    {
    case CaliPageActionType::CALI_ACTION_PREV:
    case CaliPageActionType::CALI_ACTION_RECALI:
        SetBackgroundColor(btn_bg_white);
        SetBorderColor(btn_bd_white);
        SetTextColor(btn_text_white);
        break;
    case CaliPageActionType::CALI_ACTION_START:
    case CaliPageActionType::CALI_ACTION_NEXT:
    case CaliPageActionType::CALI_ACTION_CALI:
    case CaliPageActionType::CALI_ACTION_CALI_NEXT:
    case CaliPageActionType::CALI_ACTION_FLOW_CALI_STAGE_2:
    case CaliPageActionType::CALI_ACTION_PA_SAVE:
    case CaliPageActionType::CALI_ACTION_FLOW_SAVE:
    case CaliPageActionType::CALI_ACTION_FLOW_COARSE_SAVE:
    case CaliPageActionType::CALI_ACTION_FLOW_FINE_SAVE:
    case CaliPageActionType::CALI_ACTION_COMMON_SAVE:
        SetBackgroundColor(btn_bg_green);
        SetBorderColor(btn_bd_green);
        SetTextColor(btn_text_green);
        break;
    default:
        break;
    }

    SetBackgroundColour(*wxWHITE);
    SetFont(Label::Body_13);
    SetMinSize(wxSize(-1, FromDIP(24)));
    SetCornerRadius(FromDIP(12));
}


FilamentComboBox::FilamentComboBox(wxWindow* parent, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, wxID_ANY, pos, size, wxTAB_TRAVERSAL)
{
    SetBackgroundColour(*wxWHITE);

    wxBoxSizer* main_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_comboBox = new CalibrateFilamentComboBox(this);
    m_comboBox->SetSize(CALIBRATION_FILAMENT_COMBOX_SIZE);
    m_comboBox->SetMinSize(CALIBRATION_FILAMENT_COMBOX_SIZE);
    main_sizer->Add(m_comboBox->clr_picker, 0, wxALIGN_CENTER | wxRIGHT, FromDIP(8));
    main_sizer->Add(m_comboBox, 0, wxALIGN_CENTER);

    this->SetSizer(main_sizer);
    this->Layout();
    main_sizer->Fit(this);
}

void FilamentComboBox::set_select_mode(CalibrationFilamentMode mode)
{
    m_mode = mode;
    if (m_checkBox)
        m_checkBox->Show(m_mode == CalibrationFilamentMode::CALI_MODEL_MULITI);
    if (m_radioBox)
        m_radioBox->Show(m_mode == CalibrationFilamentMode::CALI_MODEL_SINGLE);

    Layout();
}

void FilamentComboBox::load_tray_from_ams(int id, DynamicPrintConfig& tray)
{
    m_comboBox->load_tray(tray);

    m_tray_id = id;
    m_tray_name = m_comboBox->get_tray_name();
    m_is_bbl_filamnet = MachineObject::is_bbl_filament(m_comboBox->get_tag_uid());
    Enable(m_comboBox->is_tray_exist());

    if (m_comboBox->is_tray_exist()) {
        if (!m_comboBox->is_compatible_with_printer()) {
            SetValue(false);
        }

        if (m_radioBox)
            m_radioBox->Enable(m_comboBox->is_compatible_with_printer());
            
        if (m_checkBox)
            m_checkBox->Enable(m_comboBox->is_compatible_with_printer());

    }

    // check compatibility
    wxCommandEvent event(EVT_CALI_TRAY_CHANGED);
    event.SetEventObject(GetParent());
    wxPostEvent(GetParent(), event);
}

void FilamentComboBox::update_from_preset() { m_comboBox->update(); }

bool FilamentComboBox::Show(bool show)
{
    return wxPanel::Show(show);
}

bool FilamentComboBox::Enable(bool enable) {
    if (!enable)
        SetValue(false);

    if (m_radioBox)
        m_radioBox->Enable(enable);
    if (m_checkBox)
        m_checkBox->Enable(enable);
    return wxPanel::Enable(enable);
}

void FilamentComboBox::SetValue(bool value, bool send_event) {
    if (m_radioBox) {
        if (value == m_radioBox->GetValue()) {
            if (m_checkBox) {
                if (value == m_checkBox->GetValue())
                    return;
            }
            else {
                return;
            }
        }
    }
    if (m_radioBox)
        m_radioBox->SetValue(value);
    if (m_checkBox)
        m_checkBox->SetValue(value);
}



CaliPageCaption::CaliPageCaption(wxWindow* parent, CalibMode cali_mode,
    wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style)
{
    init_bitmaps();

    SetBackgroundColour(*wxWHITE);

    auto top_sizer = new wxBoxSizer(wxVERTICAL);
    auto caption_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_prev_btn = new ScalableButton(this, wxID_ANY, "cali_page_caption_prev",
        wxEmptyString, wxDefaultSize, wxDefaultPosition, wxBU_EXACTFIT | wxNO_BORDER, true, 30);
    m_prev_btn->SetBackgroundColour(*wxWHITE);
    caption_sizer->Add(m_prev_btn, 0, wxALIGN_CENTER | wxRIGHT, FromDIP(10));

    wxString title = get_cali_mode_caption_string(cali_mode);
    Label* title_text = new Label(this, title);
    title_text->SetFont(Label::Head_20);
    title_text->Wrap(-1);
    caption_sizer->Add(title_text, 0, wxALIGN_CENTER | wxRIGHT, FromDIP(10));

    m_help_btn = new ScalableButton(this, wxID_ANY, "cali_page_caption_help",
        wxEmptyString, wxDefaultSize, wxDefaultPosition, wxBU_EXACTFIT | wxNO_BORDER, true, 30);
    m_help_btn->Hide();
    m_help_btn->SetBackgroundColour(*wxWHITE);
    caption_sizer->Add(m_help_btn, 0, wxALIGN_CENTER);

    caption_sizer->AddStretchSpacer();

    m_wiki_url = get_calibration_wiki_page(cali_mode);
    create_wiki(this);
    caption_sizer->Add(m_wiki_text, 0);

    top_sizer->Add(caption_sizer, 1, wxEXPAND);
    top_sizer->AddSpacer(FromDIP(35));
    this->SetSizer(top_sizer);
    top_sizer->Fit(this);

    // hover effect
    //m_prev_btn->Bind(wxEVT_ENTER_WINDOW, [this](auto& e) {
    //    m_prev_btn->SetBitmap(m_prev_bmp_hover.bmp());
    //});

    //m_prev_btn->Bind(wxEVT_LEAVE_WINDOW, [this](auto& e) {
    //    m_prev_btn->SetBitmap(m_prev_bmp_normal.bmp());
    //});

    // hover effect
    //m_help_btn->Bind(wxEVT_ENTER_WINDOW, [this](auto& e) {
    //    m_help_btn->SetBitmap(m_help_bmp_hover.bmp());
    //    });

    //m_help_btn->Bind(wxEVT_LEAVE_WINDOW, [this](auto& e) {
    //    m_help_btn->SetBitmap(m_help_bmp_normal.bmp());
    //    });

    // send event
    m_prev_btn->Bind(wxEVT_BUTTON, [this](auto& e) {
        wxCommandEvent event(EVT_CALI_ACTION);
        event.SetEventObject(m_parent);
        event.SetInt((int)(CaliPageActionType::CALI_ACTION_GO_HOME));
        wxPostEvent(m_parent, event);
        });

#ifdef __linux__
    wxGetApp().CallAfter([this, title_text]() {
        title_text->SetMinSize(title_text->GetSize() + wxSize{ FromDIP(150), title_text->GetCharHeight() / 2 });
        Layout();
        Fit();
        });
#endif
}

void CaliPageCaption::init_bitmaps() {
    m_prev_bmp_normal = ScalableBitmap(this, "cali_page_caption_prev", 30);
    m_prev_bmp_hover = ScalableBitmap(this, "cali_page_caption_prev_hover", 30);
    m_help_bmp_normal = ScalableBitmap(this, "cali_page_caption_help", 30);
    m_help_bmp_hover = ScalableBitmap(this, "cali_page_caption_help_hover", 30);
}

void CaliPageCaption::create_wiki(wxWindow* parent)
{
    m_wiki_text = new Label(parent, _L("Wiki"));
    m_wiki_text->SetFont(Label::Head_14);
    m_wiki_text->SetForegroundColour({ 0x0058dc });
    m_wiki_text->Bind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent& e) {
        e.Skip();
        SetCursor(wxCURSOR_HAND);
        });
    m_wiki_text->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& e) {
        e.Skip();
        SetCursor(wxCURSOR_ARROW);
        });
    m_wiki_text->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent& e) {
        if (!m_wiki_url.empty())
            wxLaunchDefaultBrowser(m_wiki_url);
        });
}

void CaliPageCaption::show_prev_btn(bool show)
{
    m_prev_btn->Show(show);
}

void CaliPageCaption::show_help_icon(bool show)
{
    //m_help_btn->Show(show);
    m_help_btn->Hide();
}

void CaliPageCaption::on_sys_color_changed()
{
    m_prev_btn->msw_rescale();
}

CaliPageStepGuide::CaliPageStepGuide(wxWindow* parent, wxArrayString steps,
    wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style),
    m_steps(steps)
{
    SetBackgroundColour(*wxWHITE);

    auto top_sizer = new wxBoxSizer(wxVERTICAL);

    m_step_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_step_sizer->AddSpacer(FromDIP(90));
    for (int i = 0; i < m_steps.size(); i++) {
        Label* step_text = new Label(this, m_steps[i]);
        step_text->SetForegroundColour(wxColour("#CECECE"));
        m_text_steps.push_back(step_text);
        m_step_sizer->Add(step_text, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, FromDIP(15));
        if (i != m_steps.size() - 1) {
            auto line = new wxPanel(this, wxID_ANY, wxDefaultPosition);
            line->SetBackgroundColour(*wxBLACK);
            m_step_sizer->Add(line, 1, wxALIGN_CENTER);
        }
    }
    m_step_sizer->AddSpacer(FromDIP(90));

    top_sizer->Add(m_step_sizer, 0, wxEXPAND);
    top_sizer->AddSpacer(FromDIP(30));
    this->SetSizer(top_sizer);
    top_sizer->Fit(this);

    wxGetApp().UpdateDarkUIWin(this);
}

void CaliPageStepGuide::set_steps(int index)
{
    for (Label* text_step : m_text_steps) {
        text_step->SetForegroundColour(wxColour("#CECECE"));
    }
    m_text_steps[index]->SetForegroundColour(*wxBLACK);

    wxGetApp().UpdateDarkUIWin(this);
}

void CaliPageStepGuide::set_steps_string(wxArrayString steps)
{
    m_steps.Clear();
    m_text_steps.clear();
    m_step_sizer->Clear(true);
    m_steps = steps;
    m_step_sizer->AddSpacer(FromDIP(90));
    for (int i = 0; i < m_steps.size(); i++) {
        Label* step_text = new Label(this, m_steps[i]);
        step_text->SetForegroundColour(wxColour("#CECECE"));
        m_text_steps.push_back(step_text);
        m_step_sizer->Add(step_text, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, FromDIP(15));
        if (i != m_steps.size() - 1) {
            auto line = new wxPanel(this, wxID_ANY, wxDefaultPosition);
            line->SetBackgroundColour(*wxBLACK);
            m_step_sizer->Add(line, 1, wxALIGN_CENTER);
        }
    }
    m_step_sizer->AddSpacer(FromDIP(90));

    wxGetApp().UpdateDarkUIWin(this);

    Layout();
}


CaliPagePicture::CaliPagePicture(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) 
    : wxPanel(parent, id, pos, size, style)
{
    SetBackgroundColour(wxColour("#CECECE"));
    auto top_sizer = new wxBoxSizer(wxHORIZONTAL);
    top_sizer->AddStretchSpacer();
    m_img = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    top_sizer->Add(m_img);
    top_sizer->AddStretchSpacer();
    this->SetSizer(top_sizer);
    top_sizer->Fit(this);
}

void CaliPagePicture::set_img(const wxBitmap& bmp)
{
    m_img->SetBitmap(bmp);
}


PAPageHelpPanel::PAPageHelpPanel(wxWindow* parent, bool ground_panel, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style)
{
    if (ground_panel)
        SetBackgroundColour(wxColour("#EEEEEE"));
    else
        SetBackgroundColour(parent->GetBackgroundColour());
    int left_align_padding = ground_panel ? FromDIP(20) : 0;

    wxBoxSizer* top_sizer = new wxBoxSizer(wxVERTICAL);
    top_sizer->AddSpacer(FromDIP(10));

    auto help_text_title = new Label(this, _L("How to use calibration result?"));
    help_text_title->SetFont(Label::Head_14);
    top_sizer->Add(help_text_title, 0, wxLEFT | wxRIGHT, left_align_padding);

    wxBoxSizer* help_text_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto help_text = new Label(this, _L("You could change the Flow Dynamics Calibration Factor in material editing"));
    help_text->SetFont(Label::Body_14);
    m_help_btn = new ScalableButton(this, wxID_ANY, "cali_page_caption_help", wxEmptyString, wxDefaultSize, wxDefaultPosition, wxBU_EXACTFIT | wxNO_BORDER, false, 24);
    m_help_btn->SetBackgroundColour(m_help_btn->GetParent()->GetBackgroundColour());
    help_text_sizer->Add(help_text, 0, wxALIGN_CENTER | wxLEFT, left_align_padding);
    help_text_sizer->Add(m_help_btn, 0, wxALIGN_CENTER | wxLEFT, FromDIP(8));
    help_text_sizer->AddSpacer(FromDIP(20));

    top_sizer->Add(help_text_sizer);

    top_sizer->AddSpacer(FromDIP(6));

    create_pop_window();

    SetSizer(top_sizer);
    top_sizer->Fit(this);
}

void PAPageHelpPanel::create_pop_window()
{
    m_pop_win = new PopupWindow(this);
    m_pop_win->SetBackgroundColour(*wxWHITE);
    wxBoxSizer* pop_sizer = new wxBoxSizer(wxVERTICAL);
    m_pop_win->SetSizer(pop_sizer);

    wxStaticBitmap* img = new wxStaticBitmap(m_pop_win, wxID_ANY, wxNullBitmap);
    if (wxGetApp().app_config->get_language_code() == "zh-cn") {
        img->SetBitmap(ScalableBitmap(this, "cali_fdc_editing_diagram_CN", 206).bmp());
    } else {
        img->SetBitmap(ScalableBitmap(this, "cali_fdc_editing_diagram", 206).bmp());
    }
    pop_sizer->Add(img, 1, wxEXPAND | wxALL, FromDIP(20));

    m_pop_win->Layout();
    m_pop_win->Fit();

    m_pop_win->Bind(wxEVT_PAINT, [this](auto&) {
        wxPaintDC dc(m_pop_win);
        dc.SetPen({ 0xACACAC });
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle({ 0, 0 }, m_pop_win->GetSize());
        });

    m_help_btn->Bind(wxEVT_ENTER_WINDOW, [this](auto&) {
        wxPoint pop_pos = m_help_btn->ClientToScreen(wxPoint(0, 0));
        pop_pos.x -= FromDIP(60);
        pop_pos.y -= m_pop_win->GetSize().y + FromDIP(10);

        m_pop_win->Position(pop_pos, wxSize(0, 0));
        m_pop_win->Popup();
        });
    m_help_btn->Bind(wxEVT_LEAVE_WINDOW, [this](auto&) {
        m_pop_win->Dismiss();
        });
}


CaliPageActionPanel::CaliPageActionPanel(wxWindow* parent,
    CalibMode cali_mode,
    CaliPageType page_type,
    wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style)
{
    m_parent = parent;

    wxWindow* btn_parent = this;

    if (cali_mode == CalibMode::Calib_PA_Line) {
        if (page_type == CaliPageType::CALI_PAGE_START) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_MANAGE_RESULT));
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_MANUAL_CALI));
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_AUTO_CALI));
        }
        else if (page_type == CaliPageType::CALI_PAGE_PRESET) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_CALI));
        }
        else if (page_type == CaliPageType::CALI_PAGE_CALI) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_CALI_NEXT));
        }
        else if (page_type == CaliPageType::CALI_PAGE_PA_SAVE) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_PA_SAVE));
        }
    }
    else if (cali_mode == CalibMode::Calib_Flow_Rate) {
        if (page_type == CaliPageType::CALI_PAGE_START) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_MANUAL_CALI));
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_AUTO_CALI));
        }
        else if (page_type == CaliPageType::CALI_PAGE_PRESET) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_CALI));
        }
        else if (page_type == CaliPageType::CALI_PAGE_CALI) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_CALI_NEXT));
        }
        else if (page_type == CaliPageType::CALI_PAGE_COARSE_SAVE) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_FLOW_COARSE_SAVE));
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_FLOW_CALI_STAGE_2));
        }
        else if (page_type == CaliPageType::CALI_PAGE_FINE_SAVE) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_FLOW_FINE_SAVE));
        }
        else if (page_type == CaliPageType::CALI_PAGE_FLOW_SAVE) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_FLOW_SAVE));
        }
    }
    else {
        if (page_type == CaliPageType::CALI_PAGE_START) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_START));
        }
        else if (page_type == CaliPageType::CALI_PAGE_PRESET) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_CALI));
        }
        else if (page_type == CaliPageType::CALI_PAGE_CALI) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_CALI_NEXT));
        }
        else if (page_type == CaliPageType::CALI_PAGE_COMMON_SAVE) {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_COMMON_SAVE));
        }
        else {
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_PREV));
            m_action_btns.push_back(new CaliPageButton(btn_parent, CaliPageActionType::CALI_ACTION_NEXT));
        }
        
    }

    auto top_sizer = new wxBoxSizer(wxHORIZONTAL);

    top_sizer->Add(0, 0, 1, wxEXPAND, 0);
    for (int i = 0; i < m_action_btns.size(); i++) {
        top_sizer->Add(m_action_btns[i], 0, wxALL, FromDIP(5));

        m_action_btns[i]->Bind(wxEVT_BUTTON,
            [this, i](wxCommandEvent& evt) {
                wxCommandEvent event(EVT_CALI_ACTION);
                event.SetEventObject(m_parent);
                event.SetInt((int)m_action_btns[i]->get_action_type());
                wxPostEvent(m_parent, event);
            });
    }
    top_sizer->Add(0, 0, 1, wxEXPAND, 0);

    this->SetSizer(top_sizer);
    top_sizer->Fit(this);
}

void CaliPageActionPanel::bind_button(CaliPageActionType action_type, bool is_block)
{
    for (int i = 0; i < m_action_btns.size(); i++) {
        if (m_action_btns[i]->get_action_type() == action_type) {

            if (is_block) {
                m_action_btns[i]->Bind(wxEVT_BUTTON,
                    [this](wxCommandEvent& evt) {
                        MessageDialog msg(nullptr, _L("The current firmware version of the printer does not support calibration.\nPlease upgrade the printer firmware."), _L("Calibration not supported"), wxOK | wxICON_WARNING);
                        msg.ShowModal();
                    });
            }
            else {
                m_action_btns[i]->Bind(wxEVT_BUTTON,
                    [this, i](wxCommandEvent& evt) {
                        wxCommandEvent event(EVT_CALI_ACTION);
                        event.SetEventObject(m_parent);
                        event.SetInt((int)m_action_btns[i]->get_action_type());
                        wxPostEvent(m_parent, event);
                    });
            }
        }
    }

}

void CaliPageActionPanel::show_button(CaliPageActionType action_type, bool show)
{
    for (int i = 0; i < m_action_btns.size(); i++) {
        if (m_action_btns[i]->get_action_type() == action_type) {
            m_action_btns[i]->Show(show);
        }
    }
    Layout();
}

void CaliPageActionPanel::enable_button(CaliPageActionType action_type, bool enable)
{
    for (int i = 0; i < m_action_btns.size(); i++) {
        if (m_action_btns[i]->get_action_type() == action_type) {
            m_action_btns[i]->Enable(enable);
        }
    }
}

CalibrationWizardPage::CalibrationWizardPage(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style)
    , m_parent(parent)
{
    SetBackgroundColour(*wxWHITE);
    SetMinSize({ MIN_CALIBRATION_PAGE_WIDTH, -1 });
}

void CalibrationWizardPage::msw_rescale()
{
}

void CalibrationWizardPage::on_sys_color_changed()
{
    m_page_caption->on_sys_color_changed();
}

}}