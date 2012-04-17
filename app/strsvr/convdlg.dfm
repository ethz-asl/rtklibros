object ConvDialog: TConvDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Conversion Options'
  ClientHeight = 206
  ClientWidth = 387
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
  object LabelConv: TLabel
    Left = 192
    Top = 8
    Width = 12
    Height = 13
    Caption = '->'
    Enabled = False
  end
  object Panel1: TPanel
    Left = 0
    Top = 26
    Width = 383
    Height = 157
    BevelOuter = bvNone
    TabOrder = 0
    object Label1: TLabel
      Left = 8
      Top = 94
      Width = 48
      Height = 13
      Caption = 'Station ID'
    end
    object Label7: TLabel
      Left = 8
      Top = 116
      Width = 57
      Height = 13
      Caption = 'Lat/Lon/Hgt'
    end
    object Label8: TLabel
      Left = 8
      Top = 138
      Width = 75
      Height = 13
      Caption = 'Antenna Offset'
    end
    object Label9: TLabel
      Left = 200
      Top = 94
      Width = 68
      Height = 13
      Caption = 'Antenna Type'
    end
    object GroupBox1: TGroupBox
      Left = 200
      Top = 4
      Width = 169
      Height = 83
      Caption = 'Satellite System'
      Enabled = False
      TabOrder = 0
      object SatSys1: TCheckBox
        Left = 12
        Top = 16
        Width = 53
        Height = 17
        Caption = 'GPS'
        Checked = True
        State = cbChecked
        TabOrder = 0
      end
      object SatSys2: TCheckBox
        Left = 12
        Top = 38
        Width = 69
        Height = 17
        Caption = 'GLONASS'
        TabOrder = 1
      end
      object SatSys3: TCheckBox
        Left = 12
        Top = 60
        Width = 53
        Height = 17
        Caption = 'Galileo'
        TabOrder = 2
      end
      object SatSys4: TCheckBox
        Left = 86
        Top = 16
        Width = 53
        Height = 17
        Caption = 'QZSS'
        TabOrder = 3
      end
      object SatSys5: TCheckBox
        Left = 86
        Top = 38
        Width = 53
        Height = 17
        Caption = 'SBAS'
        TabOrder = 4
      end
      object SatSys6: TCheckBox
        Left = 86
        Top = 60
        Width = 65
        Height = 17
        Caption = 'Compass'
        Enabled = False
        TabOrder = 5
      end
    end
    object StationId: TEdit
      Left = 92
      Top = 90
      Width = 91
      Height = 21
      TabOrder = 1
    end
    object AntPos1: TEdit
      Left = 92
      Top = 112
      Width = 91
      Height = 21
      TabOrder = 2
    end
    object AntPos2: TEdit
      Left = 184
      Top = 112
      Width = 91
      Height = 21
      TabOrder = 3
    end
    object AntPos3: TEdit
      Left = 276
      Top = 112
      Width = 91
      Height = 21
      TabOrder = 4
    end
    object AntOff1: TEdit
      Left = 92
      Top = 134
      Width = 91
      Height = 21
      TabOrder = 5
    end
    object AntOff2: TEdit
      Left = 184
      Top = 134
      Width = 91
      Height = 21
      TabOrder = 6
    end
    object AntOff3: TEdit
      Left = 276
      Top = 134
      Width = 91
      Height = 21
      TabOrder = 7
    end
    object AntType: TEdit
      Left = 276
      Top = 90
      Width = 91
      Height = 21
      TabOrder = 8
    end
    object GroupBox2: TGroupBox
      Left = 6
      Top = 4
      Width = 187
      Height = 83
      Caption = 'Interval'
      Enabled = False
      TabOrder = 9
      object Label4: TLabel
        Left = 10
        Top = 16
        Width = 85
        Height = 13
        Caption = 'Observation Data'
      end
      object Label5: TLabel
        Left = 10
        Top = 38
        Width = 49
        Height = 13
        Caption = 'Ephemeris'
      end
      object Label6: TLabel
        Left = 10
        Top = 60
        Width = 64
        Height = 13
        Caption = 'Antenna Info'
      end
      object IntevalObs: TComboBox
        Left = 114
        Top = 14
        Width = 65
        Height = 21
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 0
        Text = 'All'
        Items.Strings = (
          'All'
          '1 s'
          '2 s'
          '5 s'
          '10 s'
          '15 s'
          '30 s'
          '60 s')
      end
      object IntervalEph: TComboBox
        Left = 114
        Top = 36
        Width = 65
        Height = 21
        ItemHeight = 13
        ItemIndex = 1
        TabOrder = 1
        Text = 'All'
        Items.Strings = (
          ''
          'All'
          '1 s'
          '5 s'
          '10 s'
          '30 s'
          '60 s'
          '120 s')
      end
      object IntervalAnt: TComboBox
        Left = 114
        Top = 58
        Width = 65
        Height = 21
        ItemHeight = 13
        ItemIndex = 5
        TabOrder = 2
        Text = '30 s'
        Items.Strings = (
          ''
          'All'
          '1 s'
          '5 s'
          '10 s'
          '30 s'
          '60 s'
          '120 s'
          '300 s')
      end
    end
    object BtnPos: TButton
      Left = 366
      Top = 114
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
    end
  end
  object BtnOk: TButton
    Left = 192
    Top = 182
    Width = 87
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 282
    Top = 182
    Width = 87
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object Conversion: TCheckBox
    Left = 8
    Top = 6
    Width = 79
    Height = 17
    Caption = 'Conversion'
    TabOrder = 3
    OnClick = ConversionClick
  end
  object InFormat: TComboBox
    Left = 92
    Top = 4
    Width = 93
    Height = 21
    Style = csDropDownList
    DropDownCount = 16
    Enabled = False
    ItemHeight = 0
    TabOrder = 4
  end
  object OutFormat: TComboBox
    Left = 212
    Top = 4
    Width = 93
    Height = 21
    Style = csDropDownList
    DropDownCount = 16
    Enabled = False
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 5
    Text = 'RTCM2'
    Items.Strings = (
      'RTCM2'
      'RTCM3')
  end
end
