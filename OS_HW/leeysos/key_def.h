#ifndef _KEY_DEF_H
#define _KEY_DEF_H

typedef struct _KBD_KEY_DATA
{
	BYTE type;
	BYTE key;
} KBD_KEY_DATA, *PKBD_KEY_DATA;

#define KBD_KTYPE_GENERAL		0x00 // 일반 키
#define KBD_KTYPE_FUNCTION		0x01 // 기능 키
#define KBD_KTYPE_SYSTEM		0x02 // 시스템 키
#define KBD_KTYPE_SPECIAL		0x03 // 특수 키

#define KBD_FKEY_ESC			0x00 // 0x01 - 0x0f : reserved
#define KBD_FKEY_F1				0x10
#define KBD_FKEY_F2				0x11
#define KBD_FKEY_F3				0x12
#define KBD_FKEY_F4				0x13
#define KBD_FKEY_F5				0x14
#define KBD_FKEY_F6				0x15
#define KBD_FKEY_F7				0x16
#define KBD_FKEY_F8				0x17
#define KBD_FKEY_F9				0x18
#define KBD_FKEY_F10			0x19
#define KBD_FKEY_F11			0x1a
#define KBD_FKEY_F12			0x1b // 0x1c - 0x1f : reserved
#define KBD_FKEY_CAPSLOCK		0x20
#define KBD_FKEY_LSHIFT			0x21
#define KBD_FKEY_RSHIFT			0x22
#define KBD_FKEY_LCTRL			0x23
#define KBD_FKEY_RCTRL			0x24
#define KBD_FKEY_LALT			0x25
#define KBD_FKEY_RALT			0x26
#define KBD_FKEY_BACKSPACE		0x27
#define KBD_FKEY_CHINESE		0x28
#define KBD_FKEY_KORENG			0x29
#define KBD_FKEY_RWIN			0x2a // 0x2b - 0x2f : reserved
#define KBD_FKEY_PRINTSCREEN	0x30
#define KBD_FKEY_SCROLLLOCK		0x31
#define KBD_FKEY_PAUSE			0x32
#define KBD_FKEY_INSERT			0x33
#define KBD_FKEY_HOME			0x34
#define KBD_FKEY_PAGEUP			0x35
#define KBD_FKEY_DELETE			0x36
#define KBD_FKEY_END			0x37
#define KBD_FKEY_PAGEDOWN		0x38
#define KBD_FKEY_UP				0x39
#define KBD_FKEY_DOWN			0x3a
#define KBD_FKEY_LEFT			0x3b
#define KBD_FKEY_RIGHT			0x3c // 0x3d - 0x3f : reserved
#define KBD_FKEY_NUMLOCK		0x40
#define KBD_FKEY_NUMINS			0x41
#define KBD_FKEY_NUMDEL			0x42
#define KBD_FKEY_NUMENTER		0x43
#define KBD_FKEY_NUMUP			0x44
#define KBD_FKEY_NUMDOWN		0x45
#define KBD_FKEY_NUMLEFT		0x46
#define KBD_FKEY_NUMRIGHT		0x47 // 0x48 - 0x4f : reserved

#endif