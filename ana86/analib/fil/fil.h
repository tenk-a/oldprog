/*
	module fil
 // dos��ق��̂܂܂̏ꍇ�͊�{�I�ɃG���[�R�[�hax�͕��ɔ��]���ĕԂ�
*/


/*
 * DOS�n���h���n.
 */
const int STDIN=0,STDOUT=1,STDERR=2;
I	fil_open(B F *fname, W mode);
	//ret ax:̧�٥�����  ���l(-1�`-100��^^;):dos�װ����
I	fil_close(int hdl);
	//ret ax 0:ok  ���l:dos�װ����
I	fil_creat(B F *fname, W fattr);
	//ret ax:̧�٥�����  ���l:dos�װ����
I	fil_creatnew(B F *fname, W fattr);
	//ret ax:̧�٥�����  ���l:dos�װ����
W	fil_read(I hdl, B F *buf, W siz);
	//ret ax:���ۂɓǂݍ��񂾃T�C�Y�i����dos�G���[�R�[�h�j
	//       -1�`-100��:dos�G���[�R�[�h. ������ siz �� 63K�ʂ܂łɂ��Ƃ���
W	fil_write(I hdl,B F *buf, W siz);
	//ret ax:���ۂɏ������񂾃T�C�Y�i����dos�G���[�R�[�h�j
D	fil_seek(I hdl, D ofs, I posmode);
	//arg posmode  0:̧�ِ擪  1:���Ĉʒu  2:̧�ٖ�
	//ret dx.ax  ̧�ِ擪����̵̾��. -1=�G���[��������
I	fil_dup(I hdl);
	//ret ax:�V���������  ���l:dos�G���[�R�[�h
I	fil_dup2(I hdl1, I hdl2);
	//ret ax:�V���������(hdl2)  ���l:dos�G���[�R�[�h

/*
 * �f�B�X�N(�ިڸ��(����))�֌W
 */
I	fil_delete(fname:d);cdecl
	//ret ax:0=ok  ���l:dos�G���[�R�[�h
I	fil_getattr(fname:d);cdecl
	//ret ax:̧�ّ���   ���l:dos�װ����
I	fil_setattr(fname:d, attr:w);cdecl
	//ret ax:0=ok   ���l:dos�װ����
I	fil_getdrive();cdecl
	//	ret ax:0=A: 1=B: ...
I	fil_setdrive(drv:w);cdecl
	//	drv 1=A:  2=B: ...
	//	ret ax:�ݒ肳�ꂽ�ő�h���C�u�ԍ�(0=A: 1=B: ...)
I	fil_mkdir(dir:d);cdecl
	//	ret ax:0=ok	���l:dos�G���[�R�[�h
I	fil_rmdir(dir:d);cdecl
	//	ret ax:0=ok	���l:dos�G���[�R�[�h
I	fil_setcdir(dir:d);cdecl
	//���ĥ�ިڸ�؂̎w��(��ײ�ޕύX�Ȃ�)
	//	ret ax:0=ok	���l:dos�G���[�R�[�h
I	fil_setcwd(dir:d);cdecl
	//���ĥ�ިڸ�؂̎w��(��ײ�ޕύX�L��)
	//	ret ax:0=ok	���l:dos�G���[�R�[�h
I	fil_getcdir(drv:w,dir:d);cdecl
	// �w����ײ�ނ̃J�����g�E�f�B���N�g���̎擾
	//	drv 0=����  1=A:  2=B: ...
	//	ret ax:0=ok	���l:dos�G���[�R�[�h
I	fil_getcwd(dir:d);cdecl
	// �J�����g�h���C�u�̃J�����g�E�f�B���N�g���̎擾
	//	ret ax:0=ok	���l:dos�G���[�R�[�h

/*
 * �t�@�C����������֌W
 */

endmodule
