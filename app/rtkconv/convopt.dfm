object ConvOptDialog: TConvOptDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 304
  ClientWidth = 421
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
  object Label3: TLabel
    Left = 130
    Top = 284
    Width = 31
    Height = 13
    Caption = 'Debug'
  end
  object Label8: TLabel
    Left = 6
    Top = 284
    Width = 32
    Height = 13
    Caption = 'Option'
  end
  object Label11: TLabel
    Left = 324
    Top = 210
    Width = 89
    Height = 13
    Caption = 'Excluded Satellites'
  end
  object Label9: TLabel
    Left = 10
    Top = 7
    Width = 68
    Height = 13
    Caption = 'RINEX Version'
  end
  object Label12: TLabel
    Left = 172
    Top = 7
    Width = 48
    Height = 13
    Caption = 'Station ID'
  end
  object BtnOk: TButton
    Left = 256
    Top = 280
    Width = 81
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 338
    Top = 280
    Width = 81
    Height = 23
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object GroupBox1: TGroupBox
    Left = 2
    Top = 22
    Width = 417
    Height = 189
    TabOrder = 5
    object Label1: TLabel
      Left = 8
      Top = 78
      Width = 99
      Height = 13
      Caption = 'Maker Name/#/Type'
    end
    object Label2: TLabel
      Left = 8
      Top = 12
      Width = 100
      Height = 13
      Caption = 'RunBy/Obsv/Agency'
    end
    object Label4: TLabel
      Left = 8
      Top = 100
      Width = 82
      Height = 13
      Caption = 'Rec #/Type/Vers'
    end
    object Label5: TLabel
      Left = 8
      Top = 122
      Width = 56
      Height = 13
      Caption = 'Ant #/Type'
    end
    object Label6: TLabel
      Left = 8
      Top = 144
      Width = 76
      Height = 13
      Caption = 'Approx Pos XYZ'
    end
    object Label7: TLabel
      Left = 8
      Top = 166
      Width = 76
      Height = 13
      Caption = 'Ant Delta H/E/N'
    end
    object Label10: TLabel
      Left = 8
      Top = 44
      Width = 45
      Height = 13
      Caption = 'Comment'
    end
    object Marker: TEdit
      Left = 126
      Top = 76
      Width = 95
      Height = 21
      TabOrder = 5
    end
    object RunBy: TEdit
      Left = 126
      Top = 10
      Width = 95
      Height = 21
      TabOrder = 0
    end
    object Name0: TEdit
      Left = 222
      Top = 10
      Width = 95
      Height = 21
      TabOrder = 1
    end
    object Name1: TEdit
      Left = 318
      Top = 10
      Width = 95
      Height = 21
      TabOrder = 2
    end
    object Rec2: TEdit
      Left = 318
      Top = 98
      Width = 95
      Height = 21
      TabOrder = 10
    end
    object Rec1: TEdit
      Left = 222
      Top = 98
      Width = 95
      Height = 21
      TabOrder = 9
    end
    object Rec0: TEdit
      Left = 126
      Top = 98
      Width = 95
      Height = 21
      TabOrder = 8
    end
    object Ant0: TEdit
      Left = 126
      Top = 120
      Width = 95
      Height = 21
      TabOrder = 11
    end
    object Ant1: TEdit
      Left = 222
      Top = 120
      Width = 95
      Height = 21
      TabOrder = 12
    end
    object Ant2: TEdit
      Left = 318
      Top = 120
      Width = 95
      Height = 21
      TabOrder = 13
    end
    object AppPos2: TEdit
      Left = 318
      Top = 142
      Width = 95
      Height = 21
      TabOrder = 16
      Text = '0.0000'
    end
    object AntDel2: TEdit
      Left = 318
      Top = 164
      Width = 95
      Height = 21
      TabOrder = 19
      Text = '0.0000'
    end
    object AppPos1: TEdit
      Left = 222
      Top = 142
      Width = 95
      Height = 21
      TabOrder = 15
      Text = '0.0000'
    end
    object AntDel1: TEdit
      Left = 222
      Top = 164
      Width = 95
      Height = 21
      TabOrder = 18
      Text = '0.0000'
    end
    object AppPos0: TEdit
      Left = 126
      Top = 142
      Width = 95
      Height = 21
      TabOrder = 14
      Text = '0.0000'
    end
    object AntDel0: TEdit
      Left = 126
      Top = 164
      Width = 95
      Height = 21
      TabOrder = 17
      Text = '0.0000'
    end
    object Comment0: TEdit
      Left = 126
      Top = 32
      Width = 287
      Height = 21
      TabOrder = 3
    end
    object Comment1: TEdit
      Left = 126
      Top = 54
      Width = 287
      Height = 21
      TabOrder = 4
    end
    object MarkerType: TEdit
      Left = 318
      Top = 76
      Width = 95
      Height = 21
      TabOrder = 7
    end
    object MarkerNo: TEdit
      Left = 222
      Top = 76
      Width = 95
      Height = 21
      TabOrder = 6
    end
  end
  object GroupBox3: TGroupBox
    Left = 2
    Top = 244
    Width = 161
    Height = 35
    Caption = 'Observation Types'
    TabOrder = 7
    object Obs2: TCheckBox
      Left = 48
      Top = 14
      Width = 47
      Height = 15
      Caption = 'L'
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
    object Obs3: TCheckBox
      Left = 84
      Top = 14
      Width = 37
      Height = 15
      Caption = 'D'
      Checked = True
      State = cbChecked
      TabOrder = 2
    end
    object Obs4: TCheckBox
      Left = 122
      Top = 14
      Width = 33
      Height = 15
      Caption = 'S'
      Checked = True
      State = cbChecked
      TabOrder = 3
    end
    object Obs1: TCheckBox
      Left = 10
      Top = 14
      Width = 35
      Height = 15
      Caption = 'C'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
  end
  object TraceLevel: TComboBox
    Left = 164
    Top = 280
    Width = 63
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 10
    Text = 'OFF'
    Items.Strings = (
      'OFF'
      'Level 1'
      'Level 2'
      'Level 3'
      'Level 4'
      'Level 5')
  end
  object RcvOption: TEdit
    Left = 40
    Top = 280
    Width = 83
    Height = 21
    TabOrder = 9
  end
  object GroupBox2: TGroupBox
    Left = 2
    Top = 210
    Width = 317
    Height = 35
    Caption = 'Satellite Systems'
    TabOrder = 6
    object Nav1: TCheckBox
      Left = 10
      Top = 14
      Width = 47
      Height = 15
      Caption = 'GPS'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object Nav2: TCheckBox
      Left = 56
      Top = 14
      Width = 71
      Height = 15
      Caption = 'GLO'
      TabOrder = 1
    end
    object Nav3: TCheckBox
      Left = 102
      Top = 14
      Width = 59
      Height = 15
      Caption = 'Galileo'
      TabOrder = 2
    end
    object Nav4: TCheckBox
      Left = 158
      Top = 14
      Width = 55
      Height = 15
      Caption = 'QZSS'
      TabOrder = 3
    end
    object Nav5: TCheckBox
      Left = 210
      Top = 14
      Width = 49
      Height = 15
      Caption = 'SBAS'
      TabOrder = 4
    end
    object Nav6: TCheckBox
      Left = 260
      Top = 14
      Width = 49
      Height = 15
      Caption = 'Comp'
      TabOrder = 5
    end
  end
  object GroupBox4: TGroupBox
    Left = 166
    Top = 244
    Width = 251
    Height = 35
    Caption = 'Frequencies'
    TabOrder = 8
    object Freq1: TCheckBox
      Left = 10
      Top = 14
      Width = 51
      Height = 17
      Caption = 'L1'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object Freq2: TCheckBox
      Left = 50
      Top = 14
      Width = 37
      Height = 17
      Caption = 'L2'
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
    object Freq3: TCheckBox
      Left = 90
      Top = 14
      Width = 53
      Height = 17
      Caption = 'L5'
      TabOrder = 2
    end
    object Freq4: TCheckBox
      Left = 130
      Top = 14
      Width = 45
      Height = 17
      Caption = 'L6'
      TabOrder = 3
    end
    object Freq5: TCheckBox
      Left = 170
      Top = 14
      Width = 39
      Height = 17
      Caption = 'L7'
      TabOrder = 4
    end
    object Freq6: TCheckBox
      Left = 208
      Top = 14
      Width = 39
      Height = 17
      Caption = 'L8'
      TabOrder = 5
    end
  end
  object ExSats: TEdit
    Left = 320
    Top = 224
    Width = 97
    Height = 21
    TabOrder = 11
  end
  object RnxVer: TComboBox
    Left = 84
    Top = 5
    Width = 83
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 2
  end
  object RnxFile: TCheckBox
    Left = 327
    Top = 6
    Width = 84
    Height = 17
    Caption = 'RINEX Name'
    TabOrder = 3
    OnClick = RnxFileClick
  end
  object RnxCode: TEdit
    Left = 224
    Top = 5
    Width = 95
    Height = 21
    TabOrder = 4
    Text = '0000'
  end
end
