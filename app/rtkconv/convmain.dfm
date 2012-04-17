object MainWindow: TMainWindow
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'RTKCONV'
  ClientHeight = 314
  ClientWidth = 444
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object BtnConvert: TButton
    Left = 284
    Top = 290
    Width = 78
    Height = 23
    Caption = '&Convert'
    TabOrder = 0
    OnClick = BtnConvertClick
  end
  object BtnExit: TButton
    Left = 364
    Top = 290
    Width = 78
    Height = 23
    Caption = '&Exit'
    TabOrder = 2
    OnClick = BtnExitClick
  end
  object BtnOptions: TButton
    Left = 182
    Top = 290
    Width = 78
    Height = 23
    Caption = '&Options...'
    TabOrder = 1
    OnClick = BtnOptionsClick
  end
  object Panel3: TPanel
    Left = 2
    Top = 266
    Width = 441
    Height = 23
    BevelInner = bvRaised
    BevelOuter = bvLowered
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clGray
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 5
    object Message: TLabel
      Left = 2
      Top = 4
      Width = 435
      Height = 13
      Alignment = taCenter
      AutoSize = False
    end
    object BtnAbout: TSpeedButton
      Left = 422
      Top = 4
      Width = 15
      Height = 15
      Caption = '?'
      Flat = True
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      OnClick = BtnAboutClick
    end
  end
  object Panel2: TPanel
    Left = 2
    Top = 45
    Width = 441
    Height = 220
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 4
    object LabelInFile: TLabel
      Left = 8
      Top = 2
      Width = 151
      Height = 13
      Caption = 'RTCM, RCV RAW or RINEX OBS'
    end
    object LabelOutFile: TLabel
      Left = 8
      Top = 73
      Width = 213
      Height = 13
      Caption = 'RINEX OBS/NAV/GNAV/HNAV/QNAV and SBS'
    end
    object LabelFormat: TLabel
      Left = 346
      Top = 36
      Width = 34
      Height = 13
      Caption = 'Format'
    end
    object BtnOutFileView1: TSpeedButton
      Left = 402
      Top = 87
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView1Click
    end
    object BtnOutFileView2: TSpeedButton
      Left = 402
      Top = 109
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView2Click
    end
    object BtnOutFileView4: TSpeedButton
      Left = 402
      Top = 153
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView4Click
    end
    object BtnOutFileView3: TSpeedButton
      Left = 402
      Top = 131
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView3Click
    end
    object BtnOutFileView5: TSpeedButton
      Left = 402
      Top = 175
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView5Click
    end
    object BtnOutFileView6: TSpeedButton
      Left = 402
      Top = 197
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView6Click
    end
    object LabelOutDir: TLabel
      Left = 8
      Top = 36
      Width = 81
      Height = 13
      Caption = 'Output Directory'
    end
    object BtnKey: TSpeedButton
      Left = 164
      Top = 2
      Width = 15
      Height = 13
      Caption = '?'
      Flat = True
      OnClick = BtnKeyClick
    end
    object OutFileEna4: TCheckBox
      Left = 4
      Top = 155
      Width = 20
      Height = 17
      TabOrder = 5
      OnClick = OutDirEnaClick
    end
    object OutFileEna2: TCheckBox
      Left = 4
      Top = 111
      Width = 20
      Height = 17
      Checked = True
      State = cbChecked
      TabOrder = 3
      OnClick = OutDirEnaClick
    end
    object OutFileEna1: TCheckBox
      Left = 4
      Top = 89
      Width = 20
      Height = 17
      Checked = True
      State = cbChecked
      TabOrder = 2
      OnClick = OutDirEnaClick
    end
    object BtnInFile: TButton
      Left = 420
      Top = 16
      Width = 17
      Height = 19
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnClick = BtnInFileClick
    end
    object OutFile1: TEdit
      Left = 18
      Top = 87
      Width = 383
      Height = 21
      TabOrder = 6
    end
    object OutFile2: TEdit
      Left = 18
      Top = 109
      Width = 383
      Height = 21
      TabOrder = 7
    end
    object OutFile4: TEdit
      Left = 18
      Top = 153
      Width = 383
      Height = 21
      TabOrder = 9
    end
    object Format: TComboBox
      Left = 344
      Top = 50
      Width = 95
      Height = 21
      Style = csDropDownList
      DropDownCount = 20
      ItemHeight = 13
      TabOrder = 1
      OnChange = FormatChange
    end
    object BtnOutFile4: TButton
      Left = 420
      Top = 153
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
      OnClick = BtnOutFile4Click
    end
    object BtnOutFile2: TButton
      Left = 420
      Top = 109
      Width = 17
      Height = 19
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 11
      OnClick = BtnOutFile2Click
    end
    object BtnOutFile1: TButton
      Left = 420
      Top = 87
      Width = 17
      Height = 19
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 10
      OnClick = BtnOutFile1Click
    end
    object OutFileEna3: TCheckBox
      Left = 4
      Top = 133
      Width = 20
      Height = 17
      TabOrder = 4
      OnClick = OutDirEnaClick
    end
    object OutFile3: TEdit
      Left = 18
      Top = 131
      Width = 383
      Height = 21
      TabOrder = 8
    end
    object BtnOutFile3: TButton
      Left = 420
      Top = 131
      Width = 17
      Height = 19
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 12
      OnClick = BtnOutFile3Click
    end
    object OutFileEna5: TCheckBox
      Left = 4
      Top = 177
      Width = 20
      Height = 17
      TabOrder = 14
      OnClick = OutDirEnaClick
    end
    object OutFile5: TEdit
      Left = 18
      Top = 175
      Width = 383
      Height = 21
      TabOrder = 15
    end
    object BtnOutFile5: TButton
      Left = 420
      Top = 175
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
      OnClick = BtnOutFile5Click
    end
    object OutFileEna6: TCheckBox
      Left = 4
      Top = 199
      Width = 20
      Height = 17
      TabOrder = 17
      OnClick = OutDirEnaClick
    end
    object OutFile6: TEdit
      Left = 18
      Top = 197
      Width = 383
      Height = 21
      TabOrder = 18
    end
    object BtnOutFile6: TButton
      Left = 420
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
      TabOrder = 19
      OnClick = BtnOutFile6Click
    end
    object InFile: TComboBox
      Left = 4
      Top = 16
      Width = 417
      Height = 21
      DropDownCount = 20
      ItemHeight = 13
      TabOrder = 20
      OnChange = InFileChange
    end
    object OutDirEna: TCheckBox
      Left = 4
      Top = 52
      Width = 20
      Height = 17
      Checked = True
      State = cbChecked
      TabOrder = 21
      OnClick = OutDirEnaClick
    end
    object OutDir: TEdit
      Left = 18
      Top = 50
      Width = 309
      Height = 21
      TabOrder = 22
      OnChange = OutDirChange
    end
    object BtnOutDir: TButton
      Left = 326
      Top = 51
      Width = 17
      Height = 18
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 23
      OnClick = BtnOutDirClick
    end
  end
  object Panel1: TPanel
    Left = 2
    Top = 2
    Width = 441
    Height = 42
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 3
    object LabelTimeInt: TLabel
      Left = 372
      Top = 20
      Width = 5
      Height = 13
      Caption = 's'
    end
    object BtnTime1: TSpeedButton
      Left = 122
      Top = 2
      Width = 17
      Height = 15
      Caption = '?'
      Flat = True
      OnClick = BtnTime1Click
    end
    object BtnTime2: TSpeedButton
      Left = 282
      Top = 2
      Width = 17
      Height = 15
      Caption = '?'
      Flat = True
      OnClick = BtnTime2Click
    end
    object LabelTimeUnit: TLabel
      Left = 428
      Top = 20
      Width = 7
      Height = 13
      Caption = 'H'
      Enabled = False
    end
    object TimeStartF: TCheckBox
      Left = 4
      Top = 2
      Width = 109
      Height = 17
      Caption = 'Time Start (GPST)'
      TabOrder = 0
      OnClick = TimeStartFClick
    end
    object TimeY1: TEdit
      Left = 4
      Top = 18
      Width = 65
      Height = 21
      Enabled = False
      TabOrder = 1
      Text = '2000/01/01'
    end
    object TimeY1UD: TUpDown
      Left = 68
      Top = 19
      Width = 17
      Height = 20
      Enabled = False
      Min = -32000
      Max = 32000
      TabOrder = 2
      Wrap = True
      OnChangingEx = TimeY1UDChangingEx
    end
    object TimeH1: TEdit
      Left = 86
      Top = 18
      Width = 55
      Height = 21
      Enabled = False
      TabOrder = 3
      Text = '00:00:00'
    end
    object TimeH1UD: TUpDown
      Left = 140
      Top = 19
      Width = 17
      Height = 20
      Enabled = False
      Min = -32000
      Max = 32000
      TabOrder = 4
      Wrap = True
      OnChangingEx = TimeH1UDChangingEx
    end
    object TimeEndF: TCheckBox
      Left = 164
      Top = 2
      Width = 103
      Height = 17
      Caption = 'Time End (GPST)'
      TabOrder = 5
      OnClick = TimeEndFClick
    end
    object TimeY2: TEdit
      Left = 164
      Top = 18
      Width = 65
      Height = 21
      Enabled = False
      TabOrder = 6
      Text = '2010/12/31'
    end
    object TimeY2UD: TUpDown
      Left = 228
      Top = 19
      Width = 17
      Height = 20
      Enabled = False
      Min = -32000
      Max = 32000
      TabOrder = 7
      OnChangingEx = TimeY2UDChangingEx
    end
    object TimeH2: TEdit
      Left = 246
      Top = 18
      Width = 55
      Height = 21
      Enabled = False
      TabOrder = 8
      Text = '23:59:59'
    end
    object TimeH2UD: TUpDown
      Left = 300
      Top = 19
      Width = 17
      Height = 20
      Enabled = False
      Min = -32000
      Max = 32000
      TabOrder = 9
      Wrap = True
      OnChangingEx = TimeH2UDChangingEx
    end
    object TimeIntF: TCheckBox
      Left = 324
      Top = 2
      Width = 61
      Height = 17
      Caption = 'Interval'
      TabOrder = 10
      OnClick = TimeIntFClick
    end
    object TimeInt: TComboBox
      Left = 324
      Top = 18
      Width = 45
      Height = 21
      DropDownCount = 20
      ItemHeight = 13
      TabOrder = 11
      Text = '0'
      Items.Strings = (
        '0'
        '0.05'
        '0.1'
        '0.2'
        '0.25'
        '0.5'
        '1'
        '5'
        '10'
        '15'
        '30'
        '60')
    end
    object TimeUnitF: TCheckBox
      Left = 386
      Top = 2
      Width = 51
      Height = 17
      Caption = 'Unit'
      Enabled = False
      TabOrder = 12
      OnClick = TimeIntFClick
    end
    object TimeUnit: TEdit
      Left = 386
      Top = 18
      Width = 39
      Height = 21
      Enabled = False
      TabOrder = 13
      Text = '24'
    end
  end
  object BtnPlot: TButton
    Left = 2
    Top = 290
    Width = 78
    Height = 23
    Caption = '&Plot...'
    TabOrder = 6
    OnClick = BtnPlotClick
  end
  object BtnPost: TButton
    Left = 82
    Top = 290
    Width = 78
    Height = 23
    Caption = '&Process...'
    TabOrder = 7
    OnClick = BtnPostClick
  end
  object OpenDialog: TOpenDialog
    Filter = 
      'All (*.*)|*.*|RTCM 2 (*.rtcm2)|*.rtcm2|RTCM 3 (*.rtcm3)|*.rtcm3|' +
      'NovtAtel (*.gps)|*.gps|ublox (*.ubx)|*.ubx|SuperStart II (*.log)' +
      '|*.log|Hemisphere (*.bin)|*.bin|Javad (*.jps)|*.jps|RINEX OBS (*' +
      '.obs,*.*O)|*.obs;*.*O'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Left = 150
    Top = 256
  end
  object OpenDialog2: TOpenDialog
    Filter = 
      'All (*.*)|*.*|RINEX OBS (*.obs,*.*O)|*.obs;*.*O|RINEX NAV (*.nav' +
      ',*.*N,*.*P)|*.nav;*.*N;*.*P|RINEX GNAV (*.gnav,*.*G)|*.gnav;*.*G' +
      '|RINEX HNAV (*.hnav,*.*H)|*.hnav;*.*H|RINEX QNAV (*.qnav,*.*Q)|*' +
      '.qnav;*.*Q|SBAS Log (*.sbs)|*.sbs;*.SBS|LEX Log (*.lex)|*.lex;*.' +
      'LEX'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Left = 120
    Top = 256
  end
end
