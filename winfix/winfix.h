/* winfix.h */

#ifndef WINFIX_H
#define WINFIX_H

#ifndef WINGDIAPI
#define WINGDIAPI __declspec(dllimport)
#endif
#ifndef APIENTRY
#define APIENTRY __stdcall
#endif
#ifndef CALLBACK
#define CALLBACK APIENTRY
#endif

#endif