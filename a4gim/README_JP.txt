a4gim/a4gim2���C�u���� R1.0 �����[�X�m�[�g
																		2018.08.19

�y�T�v�z																		
a4gim/a4gim2(R1.0)�́A4GIM(V1)��Arduino IDE�ŊȒP�ɗ��p���邽�߂̃��C�u�����ł�[1]�B
a3gim/a3gim2(R4.2)���x�[�X�Ƃ��A�ł��邾���݊�����ێ������܂܂�4GIM�����Ƀt�H�[�N�������̂ł��B

�@[1] a4gim�͎��Arduino UNO/Pro���̂悤�Ƀ������T�C�Y(SRAM)���������A����3GIM/4GIM�Ƃ̐ڑ���
�@�@�@�@SoftwareSerial���g�p���郍�[�G���h�ȃ}�C�R�������ł��B����Aa4gim2�͎��Arduino Mega/
�@�@�@�@Zero/m0/m0+���̂悤�Ƀ������T�C�Y(SRAM)����r�I�傫���A�n�[�h�E�F�A�V���A�����g�p�ł���~�h���G���h
�@�@�@�@�ȏ�̃}�C�R�������ł��B

�y3GIM������a3gim/a3gim2�Ƃ̈Ⴂ�z
4GIM(V1)�́A3GIM(v2.2)�ɑ΂��ĉ��L�̂悤�ȋ@�\�I�ȍ��ق�����܂��F
�@�E4GIM��GPS�@�\�������Ă��܂���
�@�E4GIM�́A4G�l�b�g���[�N���̂��猻�݂̓������擾���邱�Ƃ��ł��܂��B

��L�̍��ق���Aa3gim/a3gim2(R4.2)�ɑ΂���a4gim/a4gim2(R1.0)�͉��L�̍��ق�����܂��B
�@�ELocation�֘A�̃��\�b�h(getLocation��)�͍폜����܂����B
�@�ELocation�@�\�̑���ɁAUDP�@�\��񋟂��܂��B

�ya4gim�𗘗p����ۂ̒��ӓ_�z
�E4GIM(V1)�̃f�t�H���g�̒ʐM���x��115200bps�ł��Ba4gim�ł́A�S�f�h�l��Arduino�Ƃ̊Ԃ�UART�ʐM��
�@SoftwareSerial���g�p���܂����A115200bps�̏ꍇ�ɂ́A�ʐM���x���������ĕ�����������P�[�X������܂��B
�@���������ꍇ�ɂ́A�ʐM���x��57600��9600bps�ɗ��Ƃ����ƂŁA���肵���ʐM���\�ƂȂ�܂��B�ʐM���x�̕ύX�́A
�@�T���v���X�P�b�`setBaudrate���Q�l�ɂ��Ă��������B�Ȃ��A��x�ݒ肵���ʐM���x�͓d����؂��Ă�4GIM����
�@�L�^����܂��̂ŁA�����ӂ��������B

---
a3gim R4.2 �����[�X�m�[�g
																		2016.05.15

enterAT()�֐��̃o�O(�w�肵�����Ԃ�AT�p�X�X���[���[�h����
�߂�Ȃ��ꍇ��������)���C�����܂����B

--
a3gim2 R4.1 �����[�X�m�[�g
																		2016.05.07

�傫�ȃT�C�Y��RAM�����}�C�R���ɂ̂ݑΉ����܂����B
�{���C�u�����̎��s�ɂ́A�T��4KB�ȏ�̃T�C�Y��RAM���K�v�ł��B
RAM�T�C�Y���������Ɠ��삪�s����ƂȂ�܂��̂ŁA�����ӂ��������B

�{���C�u�������g����ŁA�ȉ��̒��ӓ_������܂��F

(1) �ŏ���connectTCP()�̌Ăяo���A���邢�͍Ō�̒ʐM����T��
�@�@30�b�ȏ�̎��Ԃ��o�߂��Ă���ꍇ�́AconnectTCP()�̌Ă�
�@�@�o���Ɏ��s���鎞������܂�(TIMEOUT�G���[��)�B
�@�@����́A��莞�ԒʐM�����Ȃ��ƁA3G�̃R�l�N�V��������U�ؒf����
�@�@����d�͂�}���铮����s�����߂ł��B
�@�@���̂��߁A�T���v���X�P�b�`sample_TCPIP�ɂ���悤�ɁA
�@�@connectTCP()�֐��̌Ăяo����3����x�܂Ń��g���C����悤��
�@�@�ڑ������̎��������肢���܂��B

--
a3gim2 R4.0 �����[�X�m�[�g
																		2016.02.28

3GIM(Ver2)�ɑΉ����܂����B
3GIM(Ver1/Ver1.1)�����g���ɂȂ�ꍇ�ɂ́A���������A
R3.1�̕������̂܂܂��g�����������B

R3.1����̕ύX�_�͉��L�̒ʂ�ł��B
 
�ȉ��̊֐��̎d�l��ύX���܂����F
  readSMS() : �Ō�̃p�����[�^�ɃX���b�g�ԍ����w��\(�ȗ���)
  setDefaultprofile() : apn,user.password�𒼐ڎw�肷��悤�ύX
  getDefaultprofile() : apn,user.password�𒼐ڎ擾����悤�ύX
 
�ȉ��̊֐���V�K�ɒǉ����܂����F
  getStatusTCP() : TCP�R�l�N�V�����̌��݂̍ŐV�󋵂��擾
  getLocation2() : �ʒu���擾�֐��̑�����
  enterAT() : AT�R�}���h�p�X�X���[���[�h�ɓ���֐�
  writeBegin() : �����ɒ���iemSewrial��ʂ��ď������݂���֐�
  getResult() : private�֐�����public�֐��ɕύX
 
�ȉ��̊֐����폜���܂����F
  put() : 3GIM(Ver2)�ɃX�g���[�W�@�\���Ȃ����ߒ񋟂ł��Ȃ�
  get() : ����
  updateProfile() : �v���t�@�C���𒼐ڐݒ�E�擾�ł��邽�߂ɕs�v�ƂȂ���
  encryptProfile() : ����

�ȉ����O���[�o���ϐ��Ƃ��Ē�`���܂����F
  a3gSerial : �������̋@�\�ł́A���̃|�[�g�ɑ΂��Ē��ړǂݏ�������

��R4.0�̉����ł́A3GIM(Ver2)�̋@�\�ύX�ɂ��傫�Ȏd�l�ύX�ƂȂ��Ă��܂��B
  �]���̃X�P�b�`�͂��̂܂܂œ��삵�Ȃ��ꍇ������܂����A���������������B
  �Ȃ��A3GIM(Ver2)�̕ύX�_�Ɋւ��ẮA3GIM(Ver2)�̃}�j���A�����Q�Ƃ��������B

--
a3gim R3.1 �����[�X�m�[�g
                                                            2015.03.22

3GIM(Ver1.1)�Ńe�X�g���s���܂����B

�ȉ���2�̊֐��𐳎��ɃT�|�[�g���܂����B
�@updateProfile()
�@encryptProfile()

--
a3gim R3.0 �����[�X�m�[�g
                                                            2014.12.10

3GIM�p��Arduino���C�u����a3gim R3.0�ł��B
�@�\�I�ɂ́Aa3gs R3.0�Ɠ����ł��B


�ya3gs�Ƃ̍��قɂ��āz

a3gim�ł́A�d�������SMS��M���̊��������݂̂��ύX�ƂȂ��Ă��܂��B
�ȉ��A�ύX�_��������܂��F

- int start(int pin)
�@�@�w�肵��pin��d������Ɏg�p���܂��B
�@�@�f�t�H���g(pin�w��Ȃ�)�ł́A�d������͍s���܂���̂�
�@�@���ON�ƂȂ�܂��B
�@�@����pin��1�ȏ�̔ԍ����w�肵�Ă��������B
�@�@���Ƃ��΁AArduino��D6��3GIM��1�ԃs���ɐڑ������ꍇ�́A
�@�@�@�@a3gs.start(6)
�@�@�̂悤�ɌĂяo���Ă��������B

- void shutdown()
�@�@�d��������s���Ă���ꍇ�A3GIM�̓d����OFF�ɂ��܂��B

- onSMSReceived()
�@�@3GIM�ł�SMS����M���Ă��������܂���B

�ya3gim���C�u�����̎g�����z

a3gim���C�u�����́AArduino UNO/Pro����3GIM�𗘗p���邽�߂̃��C�u�����ł��B
���L�̌������s���Ă��炲�g�p���������B

- Arduino D2-3,D6-13,A0-5�̂����ꂩ���͐ڑ��Ȃ�  -->  3GIM #1(PWR_SW)
- Arduino D4  -->  3GIM #3(TX)
- Arduino D5  -->  3GIM #2(RX)
- Arduino 5V  -->  3GIM #4(IOREF)
- Arduino GND -->  3GIM #6(GND)
- 3.7V�d���܂��̓��`�E���d�r�Ȃ�  --> 3GIM #5(VCC)

--
