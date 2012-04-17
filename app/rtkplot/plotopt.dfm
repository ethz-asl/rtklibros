object PlotOptDialog: TPlotOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 323
  ClientWidth = 537
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Msg: TLabel
    Left = 4
    Top = 212
    Width = 3
    Height = 13
  end
  object Panel2: TPanel
    Left = 0
    Top = 0
    Width = 537
    Height = 323
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 2
    object Label1: TLabel
      Left = 360
      Top = 123
      Width = 84
      Height = 13
      Caption = 'Background Color'
    end
    object Label2: TLabel
      Left = 360
      Top = 145
      Width = 45
      Height = 13
      Caption = 'Plot Style'
    end
    object Label4: TLabel
      Left = 186
      Top = 35
      Width = 74
      Height = 13
      Caption = 'Direction Arrow'
    end
    object Label5: TLabel
      Left = 360
      Top = 101
      Width = 47
      Height = 13
      Caption = 'Grid Color'
    end
    object Label6: TLabel
      Left = 360
      Top = 79
      Width = 50
      Height = 13
      Caption = 'Text Color'
    end
    object Label7: TLabel
      Left = 360
      Top = 57
      Width = 47
      Height = 13
      Caption = 'Line Color'
    end
    object Label9: TLabel
      Left = 12
      Top = 35
      Width = 72
      Height = 13
      Caption = 'Show Statistics'
    end
    object Label8: TLabel
      Left = 186
      Top = 167
      Width = 68
      Height = 13
      Caption = 'Y-Range (+/-)'
    end
    object Label10: TLabel
      Left = 360
      Top = 167
      Width = 45
      Height = 13
      Caption = 'Mark Size'
    end
    object Label12: TLabel
      Left = 12
      Top = 13
      Width = 59
      Height = 13
      Caption = 'Time Format'
    end
    object Label13: TLabel
      Left = 186
      Top = 145
      Width = 38
      Height = 13
      Caption = 'Auto Fit'
    end
    object Label14: TLabel
      Left = 360
      Top = 13
      Width = 87
      Height = 13
      Caption = 'Mark Color 1 (1-6)'
    end
    object Label17: TLabel
      Left = 12
      Top = 57
      Width = 46
      Height = 13
      Caption = 'Cycle-Slip'
    end
    object Label18: TLabel
      Left = 12
      Top = 123
      Width = 87
      Height = 13
      Caption = 'Elevation Mask ('#176')'
    end
    object Label19: TLabel
      Left = 186
      Top = 13
      Width = 73
      Height = 13
      Caption = 'Error Bar/Circle'
    end
    object Label16: TLabel
      Left = 12
      Top = 79
      Width = 75
      Height = 13
      Caption = 'Parity Unknown'
    end
    object Label20: TLabel
      Left = 186
      Top = 57
      Width = 57
      Height = 13
      Caption = 'Graph Label'
    end
    object Label21: TLabel
      Left = 186
      Top = 79
      Width = 70
      Height = 13
      Caption = 'Grid/Grid Label'
    end
    object Label22: TLabel
      Left = 186
      Top = 123
      Width = 25
      Height = 13
      Caption = 'Scale'
    end
    object Label23: TLabel
      Left = 186
      Top = 101
      Width = 43
      Height = 13
      Caption = 'Compass'
    end
    object Label24: TLabel
      Left = 12
      Top = 101
      Width = 49
      Height = 13
      Caption = 'Ephemeris'
    end
    object Label11: TLabel
      Left = 360
      Top = 211
      Width = 88
      Height = 13
      Caption = 'Animation Interval'
    end
    object Label25: TLabel
      Left = 12
      Top = 167
      Width = 84
      Height = 13
      Caption = 'Hide Low Satellite'
    end
    object Label3: TLabel
      Left = 186
      Top = 211
      Width = 84
      Height = 13
      Caption = 'Coordinate Origin'
    end
    object Label26: TLabel
      Left = 12
      Top = 145
      Width = 82
      Height = 13
      Caption = 'Elev Mask Patern'
    end
    object LabelRefPos: TLabel
      Left = 186
      Top = 254
      Width = 57
      Height = 13
      Caption = 'Lat/Lon/Hgt'
    end
    object Label28: TLabel
      Left = 12
      Top = 189
      Width = 68
      Height = 13
      Caption = 'Maximum DOP'
    end
    object LabelExSats: TLabel
      Left = 12
      Top = 304
      Width = 67
      Height = 13
      Caption = 'Excluded Sats'
    end
    object Label29: TLabel
      Left = 186
      Top = 190
      Width = 68
      Height = 13
      Caption = 'RT Buffer Size'
    end
    object Label27: TLabel
      Left = 360
      Top = 233
      Width = 88
      Height = 13
      Caption = 'Update Cycle (ms)'
    end
    object Label30: TLabel
      Left = 186
      Top = 276
      Width = 65
      Height = 13
      Caption = 'QC Command'
    end
    object Label31: TLabel
      Left = 12
      Top = 211
      Width = 82
      Height = 13
      Caption = 'Receiver Position'
    end
    object Label32: TLabel
      Left = 360
      Top = 35
      Width = 87
      Height = 13
      Caption = 'Mark Color 2 (1-6)'
    end
    object Panel1: TPanel
      Left = 350
      Top = 186
      Width = 161
      Height = 27
      BevelOuter = bvNone
      TabOrder = 47
      object FontLabel: TLabel
        Left = 105
        Top = 4
        Width = 52
        Height = 13
        Alignment = taRightJustify
        Caption = 'Font Name'
      end
      object LabelFont: TLabel
        Left = 11
        Top = 4
        Width = 22
        Height = 13
        Caption = 'Font'
      end
    end
    object ShowArrow: TComboBox
      Left = 276
      Top = 32
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 13
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object PlotStyle: TComboBox
      Left = 454
      Top = 142
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 42
      Text = 'Mark/Line'
      Items.Strings = (
        'Mark/Line'
        'Mark'
        'Line'
        'None')
    end
    object Origin: TComboBox
      Left = 276
      Top = 208
      Width = 77
      Height = 21
      Style = csDropDownList
      DropDownCount = 20
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 21
      Text = 'Start Pos'
      OnChange = OriginChange
      Items.Strings = (
        'Start Pos'
        'End Pos'
        'Average Pos'
        'Linear Fit Pos'
        'Base Station'
        'Lat/Lon/Hgt'
        'Auto Input'
        'Waypnt1'
        'Waypnt2'
        'Waypnt3'
        'Waypnt4'
        'Waypnt5'
        'Waypnt6'
        'Waypnt7'
        'Waypnt8'
        'Waypnt9'
        'Waypnt10')
    end
    object Color1: TPanel
      Left = 454
      Top = 121
      Width = 59
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 40
    end
    object BtnColor1: TButton
      Left = 513
      Top = 121
      Width = 16
      Height = 18
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 41
      OnClick = BtnColor1Click
    end
    object Color2: TPanel
      Left = 454
      Top = 99
      Width = 59
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clMedGray
      TabOrder = 38
    end
    object BtnColor2: TButton
      Left = 513
      Top = 99
      Width = 16
      Height = 18
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 39
      OnClick = BtnColor2Click
    end
    object Color3: TPanel
      Left = 454
      Top = 77
      Width = 59
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 36
    end
    object BtnColor3: TButton
      Left = 513
      Top = 77
      Width = 16
      Height = 18
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 37
      OnClick = BtnColor3Click
    end
    object Color4: TPanel
      Left = 454
      Top = 55
      Width = 59
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 34
    end
    object BtnColor4: TButton
      Left = 513
      Top = 55
      Width = 16
      Height = 18
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 35
      OnClick = BtnColor4Click
    end
    object ShowStats: TComboBox
      Left = 102
      Top = 32
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 1
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object TimeLabel: TComboBox
      Left = 102
      Top = 10
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 0
      Text = 'www/ssss'
      Items.Strings = (
        'www/ssss'
        'h:m:s GPST'
        'h:m:s UTC'
        'h:m:s JST')
    end
    object AutoScale: TComboBox
      Left = 276
      Top = 142
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 18
      Text = 'OFF'
      OnChange = AutoScaleChange
      Items.Strings = (
        'OFF'
        'ON')
    end
    object MColor1: TPanel
      Left = 454
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 22
      OnClick = MColorClick
    end
    object MColor2: TPanel
      Left = 466
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 23
      OnClick = MColorClick
    end
    object MColor3: TPanel
      Left = 478
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 24
      OnClick = MColorClick
    end
    object MColor4: TPanel
      Left = 490
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 25
      OnClick = MColorClick
    end
    object MColor5: TPanel
      Left = 502
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 26
      OnClick = MColorClick
    end
    object BtnFont: TButton
      Left = 513
      Top = 187
      Width = 16
      Height = 18
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 44
      OnClick = BtnFontClick
    end
    object ShowSlip: TComboBox
      Left = 102
      Top = 54
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 2
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'LG Jump'
        'LLI Flag')
    end
    object ShowErr: TComboBox
      Left = 276
      Top = 10
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 12
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'Bar/Circle'
        'Dots')
    end
    object MarkSize: TComboBox
      Left = 454
      Top = 164
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 43
      Text = '1'
      Items.Strings = (
        '1'
        '2'
        '3'
        '4'
        '5'
        '10'
        '15'
        '20')
    end
    object ShowHalfC: TComboBox
      Left = 102
      Top = 76
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 3
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object YRange: TComboBox
      Left = 276
      Top = 164
      Width = 77
      Height = 21
      DropDownCount = 20
      ItemHeight = 13
      ItemIndex = 4
      TabOrder = 19
      Text = '1'
      Items.Strings = (
        '0.05'
        '0.1'
        '0.2'
        '0.5'
        '1'
        '2'
        '5'
        '10'
        '20'
        '50'
        '100')
    end
    object ShowLabel: TComboBox
      Left = 276
      Top = 54
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 14
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object ShowGLabel: TComboBox
      Left = 276
      Top = 76
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 15
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'Grid'
        'Grid/Label')
    end
    object ShowScale: TComboBox
      Left = 276
      Top = 120
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 17
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object ShowCompass: TComboBox
      Left = 276
      Top = 98
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 16
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object ShowEph: TComboBox
      Left = 102
      Top = 98
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 4
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object GroupBox1: TGroupBox
      Left = 4
      Top = 237
      Width = 175
      Height = 60
      Caption = 'Satellite System'
      TabOrder = 10
      object NavSys1: TCheckBox
        Left = 8
        Top = 16
        Width = 45
        Height = 17
        Caption = 'GPS'
        Checked = True
        State = cbChecked
        TabOrder = 0
      end
      object NavSys2: TCheckBox
        Left = 60
        Top = 16
        Width = 69
        Height = 17
        Caption = 'GLO'
        TabOrder = 1
      end
      object NavSys5: TCheckBox
        Left = 60
        Top = 38
        Width = 57
        Height = 17
        Caption = 'SBAS'
        TabOrder = 4
      end
      object NavSys3: TCheckBox
        Left = 114
        Top = 16
        Width = 57
        Height = 17
        Caption = 'Galileo'
        TabOrder = 2
      end
      object NavSys4: TCheckBox
        Left = 8
        Top = 38
        Width = 47
        Height = 17
        Caption = 'QZSS'
        TabOrder = 3
      end
      object NavSys6: TCheckBox
        Left = 114
        Top = 38
        Width = 57
        Height = 17
        Caption = 'Comp'
        Enabled = False
        TabOrder = 5
      end
    end
    object ElMask: TComboBox
      Left = 102
      Top = 120
      Width = 77
      Height = 21
      DropDownCount = 20
      ItemHeight = 13
      TabOrder = 5
      Text = '0'
      Items.Strings = (
        '0'
        '5'
        '10'
        '15'
        '20'
        '25'
        '30'
        '35'
        '40'
        '45')
    end
    object AnimCycle: TComboBox
      Left = 454
      Top = 208
      Width = 77
      Height = 21
      ItemHeight = 13
      TabOrder = 45
      Text = '1'
      Items.Strings = (
        '1'
        '2'
        '5'
        '10'
        '20'
        '50'
        '100')
    end
    object HideLowSat: TComboBox
      Left = 102
      Top = 164
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 7
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object ElMaskP: TComboBox
      Left = 102
      Top = 142
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 6
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object RefPos1: TEdit
      Left = 256
      Top = 252
      Width = 87
      Height = 21
      TabOrder = 48
      Text = '0'
    end
    object RefPos2: TEdit
      Left = 344
      Top = 252
      Width = 87
      Height = 21
      TabOrder = 49
      Text = '0'
    end
    object RefPos3: TEdit
      Left = 432
      Top = 252
      Width = 81
      Height = 21
      TabOrder = 50
      Text = '0'
    end
    object BtnRefPos: TButton
      Left = 513
      Top = 253
      Width = 16
      Height = 18
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 51
      OnClick = BtnRefPosClick
    end
    object MaxDop: TComboBox
      Left = 102
      Top = 186
      Width = 77
      Height = 21
      DropDownCount = 20
      ItemHeight = 13
      TabOrder = 8
      Text = '30'
      Items.Strings = (
        '10'
        '30'
        '50'
        '100'
        '200'
        '500')
    end
    object ExSats: TEdit
      Left = 84
      Top = 298
      Width = 95
      Height = 21
      TabOrder = 11
    end
    object BuffSize: TEdit
      Left = 276
      Top = 186
      Width = 77
      Height = 21
      TabOrder = 20
      Text = '10800'
    end
    object RefCycle: TEdit
      Left = 454
      Top = 230
      Width = 77
      Height = 21
      TabOrder = 46
      Text = '100'
    end
    object MColor6: TPanel
      Left = 514
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 27
      OnClick = MColorClick
    end
    object QcCmd: TEdit
      Left = 256
      Top = 274
      Width = 275
      Height = 21
      TabOrder = 52
      Text = 'teqc'
    end
    object RcvPos: TComboBox
      Left = 102
      Top = 208
      Width = 77
      Height = 21
      Style = csDropDownList
      DropDownCount = 20
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 9
      Text = 'Single Solution'
      OnChange = RcvPosChange
      Items.Strings = (
        'Single Solution'
        'Lat/Lon/Hgt'
        'RINEX Header')
    end
    object MColor7: TPanel
      Left = 454
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 28
      OnClick = MColorClick
    end
    object MColor8: TPanel
      Left = 466
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 29
      OnClick = MColorClick
    end
    object MColor9: TPanel
      Left = 478
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 30
      OnClick = MColorClick
    end
    object MColor10: TPanel
      Left = 490
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 31
      OnClick = MColorClick
    end
    object MColor11: TPanel
      Left = 502
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 32
      OnClick = MColorClick
    end
    object MColor12: TPanel
      Left = 514
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      TabOrder = 33
      OnClick = MColorClick
    end
  end
  object BtnCancel: TButton
    Left = 456
    Top = 296
    Width = 75
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object BtnOK: TButton
    Left = 380
    Top = 296
    Width = 75
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOKClick
  end
  object ColorDialog: TColorDialog
    Left = 282
    Top = 292
  end
  object FontDialog: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Options = []
    Left = 254
    Top = 292
  end
  object OpenDialog: TOpenDialog
    Filter = 
      'Position File (*.pos)|*.pos|Exec File (*.exe)|*.exe|All File (*.' +
      '*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 312
    Top = 292
  end
end
