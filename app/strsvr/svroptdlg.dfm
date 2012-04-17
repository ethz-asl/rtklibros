object SvrOptDialog: TSvrOptDialog
  Left = 0
  Top = 0
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 304
  ClientWidth = 322
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 10
    Top = 10
    Width = 125
    Height = 13
    Caption = 'Server Buffer Size (bytes)'
  end
  object Label2: TLabel
    Left = 10
    Top = 32
    Width = 88
    Height = 13
    Caption = 'Server Cycle  (ms)'
  end
  object Label3: TLabel
    Left = 10
    Top = 54
    Width = 104
    Height = 13
    Caption = 'Inactive Timeout (ms)'
  end
  object Label6: TLabel
    Left = 10
    Top = 98
    Width = 145
    Height = 13
    Caption = 'Averaging Period of Rate (ms)'
  end
  object Label7: TLabel
    Left = 10
    Top = 142
    Width = 98
    Height = 13
    Caption = 'Output Debug Trace'
  end
  object Label8: TLabel
    Left = 10
    Top = 182
    Width = 112
    Height = 13
    Caption = 'Lat/Lon/Height (deg/m)'
  end
  object Label4: TLabel
    Left = 10
    Top = 218
    Width = 121
    Height = 13
    Caption = 'FTP/HTTP Local Directory'
  end
  object Label9: TLabel
    Left = 10
    Top = 120
    Width = 133
    Height = 13
    Caption = 'Output File Swap Margin (s)'
  end
  object Label5: TLabel
    Left = 10
    Top = 76
    Width = 123
    Height = 13
    Caption = 'Re-connect Interval  (ms)'
  end
  object Label10: TLabel
    Left = 10
    Top = 258
    Width = 90
    Height = 13
    Caption = 'HTTP/NTRIP Proxy'
  end
  object BtnOk: TButton
    Left = 108
    Top = 280
    Width = 95
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 206
    Top = 280
    Width = 95
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object SvrBuffSize: TEdit
    Left = 204
    Top = 8
    Width = 97
    Height = 21
    TabOrder = 2
    Text = '16384'
  end
  object SvrCycle: TEdit
    Left = 204
    Top = 30
    Width = 97
    Height = 21
    TabOrder = 3
    Text = '100'
  end
  object DataTimeout: TEdit
    Left = 204
    Top = 52
    Width = 97
    Height = 21
    TabOrder = 4
    Text = '10000'
  end
  object ConnectInterval: TEdit
    Left = 204
    Top = 74
    Width = 97
    Height = 21
    TabOrder = 5
    Text = '2000'
  end
  object AvePeriodRate: TEdit
    Left = 204
    Top = 96
    Width = 97
    Height = 21
    TabOrder = 6
    Text = '1000'
  end
  object TraceLevelS: TComboBox
    Left = 204
    Top = 140
    Width = 97
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 8
    Text = 'None'
    Items.Strings = (
      'None'
      'Level 1'
      'Level 2'
      'Level 3'
      'Level 4'
      'Level 5')
  end
  object NmeaPos2: TEdit
    Left = 106
    Top = 196
    Width = 97
    Height = 21
    TabOrder = 11
    Text = '0.000'
  end
  object NmeaPos1: TEdit
    Left = 8
    Top = 196
    Width = 97
    Height = 21
    TabOrder = 10
    Text = '0.000'
  end
  object NmeaCycle: TEdit
    Left = 204
    Top = 162
    Width = 97
    Height = 21
    TabOrder = 9
    Text = '0'
  end
  object NmeaPos3: TEdit
    Left = 204
    Top = 196
    Width = 97
    Height = 21
    TabOrder = 12
    Text = '0.000'
  end
  object BtnPos: TButton
    Left = 302
    Top = 197
    Width = 17
    Height = 19
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 13
    OnClick = BtnPosClick
  end
  object NmeaReqT: TCheckBox
    Left = 8
    Top = 162
    Width = 171
    Height = 17
    Caption = 'NMEA Request Cycle (ms)'
    TabOrder = 14
    OnClick = NmeaReqTClick
  end
  object LocalDir: TEdit
    Left = 8
    Top = 232
    Width = 293
    Height = 21
    TabOrder = 15
  end
  object BtnLocalDir: TButton
    Left = 302
    Top = 233
    Width = 17
    Height = 19
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 16
    OnClick = BtnLocalDirClick
  end
  object FileSwapMarginE: TEdit
    Left = 204
    Top = 118
    Width = 97
    Height = 21
    TabOrder = 7
    Text = '30'
  end
  object ProxyAddr: TEdit
    Left = 156
    Top = 256
    Width = 145
    Height = 21
    TabOrder = 17
  end
end
