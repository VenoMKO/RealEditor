#include "TextureImporter.h"
#include <wx/statline.h>

enum ControlElementId {
	Format = wxID_HIGHEST + 1,
	Normal,
	SRGB,
	MipGen,
	MipFilter,
	AddressX,
	AddressY
};

inline int PixelFormatToWx(EPixelFormat fmt)
{
	switch (fmt)
	{
	case PF_DXT1:
		return 0;
	case PF_DXT3:
		return 1;
	default:
	case PF_DXT5:
		return 2;
	}
}

inline EPixelFormat WxToPixelFormat(int fmt)
{
	switch (fmt)
	{
	case 0:
		return PF_DXT1;
	case 1:
		return PF_DXT3;
	default:
	case 2:
		return PF_DXT5;
	}
}

inline int TextureAddressToWx(TextureAddress address)
{
	switch (address)
	{
	case TA_Wrap:
		return 0;
	case TA_Clamp:
		return 1;
	case TA_Mirror:
		return 2;
	case TA_MAX:
	default:
		return -1;
	}
}

inline TextureAddress WxToTextureAddress(int address)
{
	switch (address)
	{
	case 0:
		return TA_Wrap;
	case 1:
		return TA_Clamp;
	case 2:
		return TA_Mirror;
	case TA_MAX:
	default:
		return TA_MAX;
	}
}

TextureImporter::TextureImporter(wxWindow* parent, EPixelFormat fmt, bool bNormal, bool bSRGB, TextureAddress addressX, TextureAddress addressY)
  : wxDialog(parent, wxID_ANY, wxT("Import options"), wxDefaultPosition, wxSize(552, 565))
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText1;
	m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("Format"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText1->Wrap(-1);
	m_staticText1->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer11->Add(m_staticText1, 0, wxALL, 5);

	wxPanel* m_panel1;
	m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText2;
	m_staticText2 = new wxStaticText(m_panel1, wxID_ANY, wxT("Select Pixel Format to convert your image to. As a rule of thumb, use 'DXT1' when your image has no transparency, otherwise 'DXT5' is your choice."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText2->Wrap(400);
	bSizer2->Add(m_staticText2, 0, wxALL, 5);

	wxArrayString PixelFormatChoices;
	PixelFormatChoices.Add("DXT1");
	PixelFormatChoices.Add("DXT3");
	PixelFormatChoices.Add("DXT5");
	PixelFormat = new wxChoice(m_panel1, ControlElementId::Format, wxDefaultPosition, wxDefaultSize, PixelFormatChoices, 0);
	PixelFormat->SetSelection(PixelFormatToWx(fmt));
	bSizer2->Add(PixelFormat, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	m_panel1->SetSizer(bSizer2);
	m_panel1->Layout();
	bSizer2->Fit(m_panel1);
	bSizer11->Add(m_panel1, 0, wxEXPAND | wxALL, 5);

	wxStaticText* m_staticText3;
	m_staticText3 = new wxStaticText(this, wxID_ANY, wxT("Color"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText3->Wrap(-1);
	m_staticText3->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer11->Add(m_staticText3, 0, wxALL, 5);

	wxPanel* m_panel2;
	m_panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxVERTICAL);

	Normal = new wxCheckBox(m_panel2, ControlElementId::Normal, wxT("Normal map"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer3->Add(Normal, 0, wxALL, 5);
	Normal->SetValue(bNormal);
	Normal->Enable(!bSRGB);

	wxStaticText* m_staticText7;
	m_staticText7 = new wxStaticText(m_panel2, wxID_ANY, wxT("Normal maps are compressed differently. Check this option if you are importing a normal map."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText7->Wrap(515);
	bSizer3->Add(m_staticText7, 0, wxALL, 5);

	wxStaticLine* m_staticline1;
	m_staticline1 = new wxStaticLine(m_panel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer3->Add(m_staticline1, 0, wxEXPAND | wxALL, 5);

	SRGB = new wxCheckBox(m_panel2, ControlElementId::SRGB, wxT("sRGB"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer3->Add(SRGB, 0, wxALL, 5);
	SRGB->SetValue(bSRGB);
	SRGB->Enable(!bNormal);

	wxStaticText* m_staticText8;
	m_staticText8 = new wxStaticText(m_panel2, wxID_ANY, wxT("Check this if your image uses sRGB color space. This option is unavailable for normal maps."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText8->Wrap(-1);
	bSizer3->Add(m_staticText8, 0, wxALL, 5);


	m_panel2->SetSizer(bSizer3);
	m_panel2->Layout();
	bSizer3->Fit(m_panel2);
	bSizer11->Add(m_panel2, 0, wxEXPAND | wxALL, 5);

	wxStaticText* m_staticText14;
	m_staticText14 = new wxStaticText(this, wxID_ANY, wxT("Mipmaps"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText14->Wrap(-1);
	m_staticText14->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer11->Add(m_staticText14, 0, wxALL, 5);

	wxPanel* m_panel9;
	m_panel9 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer(wxVERTICAL);

	GenMips = new wxCheckBox(m_panel9, ControlElementId::MipGen, wxT("Generate mipmaps"), wxDefaultPosition, wxDefaultSize, 0);
	GenMips->SetValue(false);
	GenMips->Enable(false);
	bSizer14->Add(GenMips, 0, wxALL, 5);

	wxStaticText* m_staticText20;
	m_staticText20 = new wxStaticText(m_panel9, wxID_ANY, wxT("Generating mipmaps during import saves GPU time during the gameplay."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText20->Wrap(-1);
	bSizer14->Add(m_staticText20, 0, wxALL, 5);

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText15;
	m_staticText15 = new wxStaticText(m_panel9, wxID_ANY, wxT("Mipmaps are rendered on distant objects to reduce GPU cost and during texture loading."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText15->Wrap(350);
	bSizer20->Add(m_staticText15, 0, wxALL, 5);

	wxStaticText* m_staticText16;
	m_staticText16 = new wxStaticText(m_panel9, wxID_ANY, wxT("Method:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText16->Wrap(-1);
	m_staticText16->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	bSizer20->Add(m_staticText16, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxArrayString MipGenMethodChoices;
	MipGenMethodChoices.Add("Box");
	MipGenMethodChoices.Add("Triangle");
	MipGenMethodChoices.Add("Kaiser");
	MipFilter = new wxChoice(m_panel9, ControlElementId::MipFilter, wxDefaultPosition, wxDefaultSize, MipGenMethodChoices, 0);
	MipFilter->SetSelection(2);
	MipFilter->Enable(GenMips->GetValue());
	bSizer20->Add(MipFilter, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	bSizer14->Add(bSizer20, 1, wxEXPAND, 0);


	m_panel9->SetSizer(bSizer14);
	m_panel9->Layout();
	bSizer14->Fit(m_panel9);
	bSizer11->Add(m_panel9, 1, wxEXPAND | wxALL, 5);


	wxStaticText* m_staticText9;
	m_staticText9 = new wxStaticText(this, wxID_ANY, wxT("Address Mode"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText9->Wrap(-1);
	m_staticText9->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer11->Add(m_staticText9, 0, wxALL, 5);

	wxPanel* m_panel3;
	m_panel3 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText10;
	m_staticText10 = new wxStaticText(m_panel3, wxID_ANY, wxT("Address mode defines the game's texture sampler behavior. Generally, you don't want to change this unless you know what you are doing."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText10->Wrap(330);
	bSizer4->Add(m_staticText10, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxPanel* m_panel4;
	m_panel4 = new wxPanel(m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText11;
	m_staticText11 = new wxStaticText(m_panel4, wxID_ANY, wxT("Address X:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText11->Wrap(-1);
	bSizer6->Add(m_staticText11, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxArrayString AddressXChoices;
	AddressXChoices.Add("Wrap");
	AddressXChoices.Add("Clamp");
	AddressXChoices.Add("Mirror");
	AddressX = new wxChoice(m_panel4, ControlElementId::AddressX, wxDefaultPosition, wxDefaultSize, AddressXChoices, 0);
	AddressX->SetSelection(TextureAddressToWx(addressX));
	AddressX->Enable(false);
	bSizer6->Add(AddressX, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	bSizer5->Add(bSizer6, 0, wxEXPAND, 0);

	wxBoxSizer* bSizer61;
	bSizer61 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText111;
	m_staticText111 = new wxStaticText(m_panel4, wxID_ANY, wxT("Address Y:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText111->Wrap(-1);
	bSizer61->Add(m_staticText111, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxArrayString AddressYChoices;
	AddressYChoices.Add("Wrap");
	AddressYChoices.Add("Clamp");
	AddressYChoices.Add("Mirror");
	AddressY = new wxChoice(m_panel4, ControlElementId::AddressY, wxDefaultPosition, wxDefaultSize, AddressYChoices, 0);
	AddressY->SetSelection(TextureAddressToWx(addressY));
	AddressY->Enable(false);
	bSizer61->Add(AddressY, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	bSizer5->Add(bSizer61, 0, wxEXPAND, 0);


	m_panel4->SetSizer(bSizer5);
	m_panel4->Layout();
	bSizer5->Fit(m_panel4);
	bSizer4->Add(m_panel4, 1, wxALL, 0);


	m_panel3->SetSizer(bSizer4);
	m_panel3->Layout();
	bSizer4->Fit(m_panel3);
	bSizer11->Add(m_panel3, 0, wxEXPAND | wxALL, 5);


	bSizer1->Add(bSizer11, 1, wxEXPAND, 0);

	wxPanel* m_panel5;
	m_panel5 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* m_panel6;
	m_panel6 = new wxPanel(m_panel5, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	bSizer10->Add(m_panel6, 1, wxEXPAND | wxALL, 5);

	ImportButton = new wxButton(m_panel5, wxID_OK, wxT("Import"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer10->Add(ImportButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	ImportButton->SetFocus();

	CancelButton = new wxButton(m_panel5, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer10->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	m_panel5->SetSizer(bSizer10);
	m_panel5->Layout();
	bSizer10->Fit(m_panel5);
	bSizer1->Add(m_panel5, 1, wxEXPAND | wxALL, 5);


	SetSizer(bSizer1);
	Layout();

	Centre(wxBOTH);
}

EPixelFormat TextureImporter::GetPixelFormat() const
{
	return WxToPixelFormat(PixelFormat->GetSelection());
}

TextureAddress TextureImporter::GetAddressX() const
{
	return WxToTextureAddress(AddressX->GetSelection());
}

TextureAddress TextureImporter::GetAddressY() const
{
	return WxToTextureAddress(AddressY->GetSelection());
}

MipFilterType TextureImporter::GetMipFilter() const
{
	return (MipFilterType)MipFilter->GetSelection();
}

bool TextureImporter::IsNormal() const
{
	return Normal->GetValue();
}

bool TextureImporter::IsSRGB() const
{
	return SRGB->GetValue();
}

bool TextureImporter::GetGenerateMips() const
{
	return GenMips->GetValue();
}

void TextureImporter::OnNormalClick(wxCommandEvent&)
{
	SRGB->Enable(!Normal->GetValue());
}

void TextureImporter::OnGenMipsClicked(wxCommandEvent&)
{
	MipFilter->Enable(GenMips->GetValue());
}

void TextureImporter::OnSRGBClick(wxCommandEvent&)
{
	Normal->Enable(!SRGB->GetValue());
}

wxBEGIN_EVENT_TABLE(TextureImporter, wxDialog)
EVT_CHECKBOX(ControlElementId::Normal, TextureImporter::OnNormalClick)
EVT_CHECKBOX(ControlElementId::MipGen, TextureImporter::OnGenMipsClicked)
EVT_CHECKBOX(ControlElementId::SRGB, TextureImporter::OnSRGBClick)
wxEND_EVENT_TABLE()