object MainForm: TMainForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Online GNSS Data Downloader'
  ClientHeight = 374
  ClientWidth = 550
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 187
    Height = 374
    Align = alLeft
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 0
    object Label1: TLabel
      Left = 6
      Top = 2
      Width = 113
      Height = 13
      Caption = 'Data/Products (Source)'
    end
    object DataTypeS: TComboBox
      Left = 2
      Top = 16
      Width = 182
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 0
      OnChange = DataTypeSChange
    end
    object DataListS: TListBox
      Left = 2
      Top = 36
      Width = 181
      Height = 337
      ItemHeight = 13
      TabOrder = 1
    end
  end
  object Panel2: TPanel
    Left = 188
    Top = 0
    Width = 361
    Height = 301
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 1
    object Label2: TLabel
      Left = 12
      Top = 2
      Width = 39
      Height = 13
      Caption = 'Address'
    end
    object Label4: TLabel
      Left = 10
      Top = 100
      Width = 50
      Height = 13
      Caption = 'Login User'
    end
    object Label5: TLabel
      Left = 10
      Top = 122
      Width = 46
      Height = 13
      Caption = 'Password'
    end
    object Label6: TLabel
      Left = 12
      Top = 264
      Width = 71
      Height = 13
      Caption = 'Local Directory'
    end
    object Label7: TLabel
      Left = 186
      Top = 82
      Width = 39
      Height = 13
      Caption = 'Stations'
    end
    object AddressE: TEdit
      Left = 4
      Top = 16
      Width = 353
      Height = 21
      TabOrder = 0
    end
    object LoginUser: TEdit
      Left = 72
      Top = 98
      Width = 109
      Height = 21
      TabOrder = 1
      Text = 'anonymous'
    end
    object Passwd: TEdit
      Left = 72
      Top = 120
      Width = 109
      Height = 21
      TabOrder = 2
      Text = 'user@'
    end
    object LocalDir: TEdit
      Left = 8
      Top = 278
      Width = 333
      Height = 21
      TabOrder = 3
    end
    object Memo1: TMemo
      Left = 184
      Top = 96
      Width = 173
      Height = 181
      ScrollBars = ssVertical
      TabOrder = 5
    end
    object GroupBox1: TGroupBox
      Left = 4
      Top = 144
      Width = 177
      Height = 119
      Caption = 'Options'
      TabOrder = 6
      object UnzipC: TCheckBox
        Left = 4
        Top = 16
        Width = 133
        Height = 17
        Caption = 'Unzip/Uncompact Files'
        TabOrder = 0
      end
      object SkipExist: TCheckBox
        Left = 4
        Top = 36
        Width = 147
        Height = 17
        Caption = 'Skip Exisiting Files'
        TabOrder = 1
      end
      object AbortOnErr: TCheckBox
        Left = 4
        Top = 56
        Width = 151
        Height = 17
        Caption = 'Abort on Download Error'
        TabOrder = 2
      end
      object ProxyEna: TCheckBox
        Left = 4
        Top = 76
        Width = 57
        Height = 17
        Caption = 'Proxy'
        TabOrder = 3
      end
      object Proxy: TEdit
        Left = 4
        Top = 94
        Width = 169
        Height = 21
        Enabled = False
        TabOrder = 4
      end
    end
    object BtnLocalDir: TButton
      Left = 340
      Top = 278
      Width = 17
      Height = 19
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 4
      OnClick = BtnLocalDirClick
    end
    object GroupBox2: TGroupBox
      Left = 4
      Top = 38
      Width = 353
      Height = 43
      Caption = 'Time Span (GPST)'
      TabOrder = 7
      object BtnTime1: TSpeedButton
        Left = 152
        Top = 14
        Width = 15
        Height = 21
        Caption = '?'
        Flat = True
      end
      object Label9: TLabel
        Left = 174
        Top = 18
        Width = 4
        Height = 13
        Caption = '-'
      end
      object BtnTime2: TSpeedButton
        Left = 332
        Top = 14
        Width = 15
        Height = 21
        Caption = '?'
        Flat = True
      end
      object TimeY1: TEdit
        Left = 4
        Top = 16
        Width = 67
        Height = 21
        TabOrder = 0
        Text = '2000/01/01'
      end
      object TimeY1UD: TUpDown
        Left = 72
        Top = 14
        Width = 15
        Height = 21
        TabOrder = 1
        OnChangingEx = TimeY1UDChangingEx
      end
      object TimeH1: TEdit
        Left = 94
        Top = 16
        Width = 39
        Height = 21
        TabOrder = 2
        Text = '00:00'
      end
      object TimeH1UD: TUpDown
        Left = 134
        Top = 14
        Width = 15
        Height = 21
        TabOrder = 3
        OnChangingEx = TimeH1UDChangingEx
      end
      object TimeY2: TEdit
        Left = 186
        Top = 16
        Width = 67
        Height = 21
        TabOrder = 4
        Text = '2000/01/01'
      end
      object TimeY2UD: TUpDown
        Left = 254
        Top = 14
        Width = 15
        Height = 21
        TabOrder = 5
        OnChangingEx = TimeY2UDChangingEx
      end
      object TimeH2: TEdit
        Left = 274
        Top = 16
        Width = 39
        Height = 21
        TabOrder = 6
        Text = '00:00'
      end
      object TimeH2UD: TUpDown
        Left = 314
        Top = 14
        Width = 15
        Height = 21
        TabOrder = 7
        OnChangingEx = TimeH2UDChangingEx
      end
    end
  end
  object Panel3: TPanel
    Left = 189
    Top = 348
    Width = 360
    Height = 25
    BevelOuter = bvNone
    TabOrder = 2
    object BtnStations: TButton
      Left = 70
      Top = 1
      Width = 71
      Height = 23
      Caption = 'Stations'
      TabOrder = 0
      OnClick = BtnStationsClick
    end
    object BtnLog: TButton
      Left = 143
      Top = 1
      Width = 71
      Height = 23
      Caption = '&Log'
      TabOrder = 1
      OnClick = BtnLogClick
    end
    object BtnDownload: TButton
      Left = 216
      Top = 1
      Width = 71
      Height = 23
      Caption = '&Download'
      TabOrder = 2
      OnClick = BtnDownloadClick
    end
    object BtnExit: TButton
      Left = 288
      Top = 1
      Width = 71
      Height = 23
      Caption = '&Exit'
      TabOrder = 3
      OnClick = BtnExitClick
    end
    object Button1: TButton
      Left = -2
      Top = 1
      Width = 71
      Height = 23
      Caption = 'Files'
      TabOrder = 4
    end
  end
  object Panel4: TPanel
    Left = 188
    Top = 302
    Width = 361
    Height = 45
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 3
  end
end
