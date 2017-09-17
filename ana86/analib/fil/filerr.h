#define NoError 0   	    	    	/* ����I�� */
#define InvalidFunctionCode -1	    	/* �����ȃt�@���N�V�����R�[�h */
#define FileNotFound -2     	    	/* �t�@�C������������Ȃ� */
#define PathNotFound -3     	    	/* �p�X����������Ȃ� */
#define TooManyOpenFiles -4 	    	/* �I�[�v���t�@�C���ߑ� */
#define AccessDenied -5     	    	/* �A�N�Z�X�ł��Ȃ� */
#define InvalidHandle -6    	    	/* �����ȃn���h�� */
#define MemoryControlBlocksDestroyed -7 /* �������R���g���[���u���b�N�j�� */
#define InsufficientMemory -8	    	/* �������s�� */
#define InvalidMemoryBlockAddress -9	/* �����ȃ������u���b�N�A�h���X */
#define InvalidEnvironment -10	    	/* �����Ȋ� */
#define InvalidFormat -11   	    	/* �����ȏ��� */
#define InvalidAccessCode -12	    	/* �����ȃA�N�Z�X�R�[�h */
#define InvalidData -13     	    	/* �����ȃf�[�^ */
#define InvalidDrive -15    	    	/* �����ȃh���C�u�� */
#define AttemptToRemoveCurrentDirectory -16
    	    	    	    /* �J�����g�f�B���N�g�����폜���悤�Ƃ��� */
#define NotSameDevice -17   	    	/* �����f�o�C�X�ł͂Ȃ� */
#define NoMoreFiles -18     	    	/* ����ȏ�t�@�C���͂Ȃ� */
#define DiskIsWriteProtected -19    	/* �f�B�X�N�����C�g�v���e�N�g��� */
#define BadDiskUnit -20     	    	/* �f�B�X�N���j�b�g�s�� */
#define DriveNotReady -21   	    	/* �h���C�u����������Ă��Ȃ� */
#define InvalidDiskCommand -22	    	/* �����ȃf�B�X�N�R�}���h */
#define CrcError -23	    	    	/* CRC �G���[ */
#define InvalidLength -24   	    	/* �����Ȓ��� */
#define SeekError -25	    	    	/* �V�[�N�G���[ */
#define NotAnMsdosDisk -26  	    	/* MS-DOS �̃f�B�X�N�ł͂Ȃ� */
#define SectorNotFound -27  	    	/* �Z�N�^��������Ȃ� */
#define OutOfPaper -28	    	    	/* ���؂� */
#define WriteFault -29	    	    	/* �������ݎ��s */
#define ReadFault -30	    	    	/* �ǂݏo�����s */
#define GeneralFailure -31  	    	/* �ʏ�̎��s */
#define ShareingViolation -32	    	/* �V�F�A�����O�ᔽ */
#define LockViolation -33   	    	/* ���b�N�ᔽ */
#define WrongDisk -34	    	    	/* �f�B�X�N�w��̎��s */
#define FcbUnavailable -35  	    	/* FCB �g�p�s�\ */
#define NetworkRequestNotSupported -50
    	    	    	    /* �l�b�g���[�N���N�G�X�g����������Ă��Ȃ� */
#define RemoteComputerNotListening -51
    	    	    	    /* �����[�g�R���s���[�^�� LISTEN ���Ă��Ȃ� */
#define DuplicateNameOnNetwork -52  	/* �l�b�g���[�N���� 2 �d��` */
#define NetworkNameNotFound -53     	/* �l�b�g���[�N����������Ȃ� */
#define NetworkBusy -54     	    	/* �l�b�g���[�N�r�W�[ */
#define NetworkDeviceNoLongerExists -55
    	    	    	    /* �l�b�g���[�N�f�o�C�X�͂���ȏ�Ȃ� */
#define NetBiosCommandLimitExceeded -56 /* �l�b�g���[�N BIOS �̌��E���z���� */
#define NetworkAdapterHardwareError -57
    	    	    	    /* �l�b�g���[�N�A�_�v�^�̃n�[�h�G���[ */
#define IncorrectResponseFromNetwork -58
    	    	    	    /* �l�b�g���[�N����̕s���ȉ��� */
#define UnexpectedNetworkError -59  	/* �\���ł��Ȃ��l�b�g���[�N�G���[ */
#define IncompatibleRemoteAdapter -60	/* �����[�g�A�_�v�^�����v���Ȃ� */
#define PrintQueueFull -61  	    	/* �v�����g�҂��s�񂪈�t */
#define QueueNotFull -62    	    	/* �҂��s��͈�t�ł͂Ȃ� */
#define NotEnoughSpaceForPrintFile -63
    	    	    	    /* �v�����g�t�@�C���̂��߂̃X�y�[�X���s�\�� */
#define NetworkNameWasDeleted -64
    	    	    	    /* �l�b�g���[�N���͊��ɍ폜����Ă��� */
#define AccessDenied2 -65   	    	/* �A�N�Z�X�ł��Ȃ� */
#define NetworkDeviceTypeIncorrect -66
    	    	    	    /* �l�b�g���[�N�f�o�C�X�̃^�C�v���s�� */
#define NetworkNameNotFound2 -67    	/* �l�b�g���[�N����������Ȃ� */
#define NetworkNameLimitExceeded -68	/* �l�b�g���[�N���̌��E���z���� */
#define NetBiosSessionLimitExceeded -69
    	    	    	    /* �l�b�g���[�N BIOS �Z�b�V�����̌��E���z���� */
#define TemporarilyPaused -70	    	/* �ꎞ�x�~ */
#define NetworkRequestNotAccepted -71
    	    	    	    /* �l�b�g���[�N�̗v�����󂯂����Ȃ� */
#define PrintOrDiskRedirectionIsPaused -72
    	    	    	    /* �v�����^,�f�B�X�N�̃��f�B���N�V�����x�~ */
#define FileExists -80	    	    	/* �t�@�C�������݂��� */
#define CannotMake -82	    	    	/* �쐬�s�\ */
#define Interrupt24hFailure -83     	/* ���荞�݃^�C�v 24H �̎��s */
#define OutOfStructures -84 	    	/* �X�g���N�`���̕s�� */
#define AlreadyAssigned -85 	    	/* ���蓖�čς� */
#define InvalidPassword -86 	    	/* �����ȃp�X���[�h */
#define InvalidParameter -87	    	/* �����ȃp�����[�^ */
#define NetWriteFault -88   	    	/* �l�b�g���[�N�ւ̏������ݎ��s */
