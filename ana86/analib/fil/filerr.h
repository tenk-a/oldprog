#define NoError 0   	    	    	/* 正常終了 */
#define InvalidFunctionCode -1	    	/* 無効なファンクションコード */
#define FileNotFound -2     	    	/* ファイル名が見つからない */
#define PathNotFound -3     	    	/* パス名が見つからない */
#define TooManyOpenFiles -4 	    	/* オープンファイル過多 */
#define AccessDenied -5     	    	/* アクセスできない */
#define InvalidHandle -6    	    	/* 無効なハンドル */
#define MemoryControlBlocksDestroyed -7 /* メモリコントロールブロック破損 */
#define InsufficientMemory -8	    	/* メモリ不足 */
#define InvalidMemoryBlockAddress -9	/* 無効なメモリブロックアドレス */
#define InvalidEnvironment -10	    	/* 無効な環境 */
#define InvalidFormat -11   	    	/* 無効な書式 */
#define InvalidAccessCode -12	    	/* 無効なアクセスコード */
#define InvalidData -13     	    	/* 無効なデータ */
#define InvalidDrive -15    	    	/* 無効なドライブ名 */
#define AttemptToRemoveCurrentDirectory -16
    	    	    	    /* カレントディレクトリを削除しようとした */
#define NotSameDevice -17   	    	/* 同じデバイスではない */
#define NoMoreFiles -18     	    	/* これ以上ファイルはない */
#define DiskIsWriteProtected -19    	/* ディスクがライトプロテクト状態 */
#define BadDiskUnit -20     	    	/* ディスクユニット不良 */
#define DriveNotReady -21   	    	/* ドライブが準備されていない */
#define InvalidDiskCommand -22	    	/* 無効なディスクコマンド */
#define CrcError -23	    	    	/* CRC エラー */
#define InvalidLength -24   	    	/* 無効な長さ */
#define SeekError -25	    	    	/* シークエラー */
#define NotAnMsdosDisk -26  	    	/* MS-DOS のディスクではない */
#define SectorNotFound -27  	    	/* セクタが見つからない */
#define OutOfPaper -28	    	    	/* 紙切れ */
#define WriteFault -29	    	    	/* 書き込み失敗 */
#define ReadFault -30	    	    	/* 読み出し失敗 */
#define GeneralFailure -31  	    	/* 通常の失敗 */
#define ShareingViolation -32	    	/* シェアリング違反 */
#define LockViolation -33   	    	/* ロック違反 */
#define WrongDisk -34	    	    	/* ディスク指定の失敗 */
#define FcbUnavailable -35  	    	/* FCB 使用不可能 */
#define NetworkRequestNotSupported -50
    	    	    	    /* ネットワークリクエストが準備されていない */
#define RemoteComputerNotListening -51
    	    	    	    /* リモートコンピュータが LISTEN していない */
#define DuplicateNameOnNetwork -52  	/* ネットワーク名の 2 重定義 */
#define NetworkNameNotFound -53     	/* ネットワーク名が見つからない */
#define NetworkBusy -54     	    	/* ネットワークビジー */
#define NetworkDeviceNoLongerExists -55
    	    	    	    /* ネットワークデバイスはこれ以上ない */
#define NetBiosCommandLimitExceeded -56 /* ネットワーク BIOS の限界を越えた */
#define NetworkAdapterHardwareError -57
    	    	    	    /* ネットワークアダプタのハードエラー */
#define IncorrectResponseFromNetwork -58
    	    	    	    /* ネットワークからの不当な応答 */
#define UnexpectedNetworkError -59  	/* 予期できないネットワークエラー */
#define IncompatibleRemoteAdapter -60	/* リモートアダプタが合致しない */
#define PrintQueueFull -61  	    	/* プリント待ち行列が一杯 */
#define QueueNotFull -62    	    	/* 待ち行列は一杯ではない */
#define NotEnoughSpaceForPrintFile -63
    	    	    	    /* プリントファイルのためのスペースが不十分 */
#define NetworkNameWasDeleted -64
    	    	    	    /* ネットワーク名は既に削除されている */
#define AccessDenied2 -65   	    	/* アクセスできない */
#define NetworkDeviceTypeIncorrect -66
    	    	    	    /* ネットワークデバイスのタイプが不当 */
#define NetworkNameNotFound2 -67    	/* ネットワーク名が見つからない */
#define NetworkNameLimitExceeded -68	/* ネットワーク名の限界を越えた */
#define NetBiosSessionLimitExceeded -69
    	    	    	    /* ネットワーク BIOS セッションの限界を越えた */
#define TemporarilyPaused -70	    	/* 一時休止 */
#define NetworkRequestNotAccepted -71
    	    	    	    /* ネットワークの要求が受けつけられない */
#define PrintOrDiskRedirectionIsPaused -72
    	    	    	    /* プリンタ,ディスクのリディレクション休止 */
#define FileExists -80	    	    	/* ファイルが存在する */
#define CannotMake -82	    	    	/* 作成不能 */
#define Interrupt24hFailure -83     	/* 割り込みタイプ 24H の失敗 */
#define OutOfStructures -84 	    	/* ストラクチャの不良 */
#define AlreadyAssigned -85 	    	/* 割り当て済み */
#define InvalidPassword -86 	    	/* 無効なパスワード */
#define InvalidParameter -87	    	/* 無効なパラメータ */
#define NetWriteFault -88   	    	/* ネットワークへの書き込み失敗 */
