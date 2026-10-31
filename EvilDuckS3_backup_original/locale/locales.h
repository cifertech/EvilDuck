#pragma once

#include "locale_types.h"
#include "locale_us.h"
#include "locale_de.h"

extern hid_locale_t locale_us;
extern hid_locale_t locale_de;

#if __has_include("locale_gb.h")
#include "locale_gb.h"
extern hid_locale_t locale_gb;
#endif

#if __has_include("locale_es.h")
#include "locale_es.h"
extern hid_locale_t locale_es;
#endif
#if __has_include("locale_fr.h")
#include "locale_fr.h"
extern hid_locale_t locale_fr;
#endif
#if __has_include("locale_ru.h")
#include "locale_ru.h"
extern hid_locale_t locale_ru;
#endif
#if __has_include("locale_dk.h")
#include "locale_dk.h"
extern hid_locale_t locale_dk;
#endif
#if __has_include("locale_be.h")
#include "locale_be.h"
extern hid_locale_t locale_be;
#endif
#if __has_include("locale_pt.h")
#include "locale_pt.h"
extern hid_locale_t locale_pt;
#endif
#if __has_include("locale_it.h")
#include "locale_it.h"
extern hid_locale_t locale_it;
#endif
#if __has_include("locale_sk.h")
#include "locale_sk.h"
extern hid_locale_t locale_sk;
#endif
#if __has_include("locale_cz.h")
#include "locale_cz.h"
extern hid_locale_t locale_cz;
#endif
#if __has_include("locale_si.h")
#include "locale_si.h"
extern hid_locale_t locale_si;
#endif
#if __has_include("locale_bg.h")
#include "locale_bg.h"
extern hid_locale_t locale_bg;
#endif
#if __has_include("locale_cafr.h")
#include "locale_cafr.h"
extern hid_locale_t locale_cafr;
#endif
#if __has_include("locale_chde.h")
#include "locale_chde.h"
extern hid_locale_t locale_chde;
#endif
#if __has_include("locale_chfr.h")
#include "locale_chfr.h"
extern hid_locale_t locale_chfr;
#endif
#if __has_include("locale_hu.h")
#include "locale_hu.h"
extern hid_locale_t locale_hu;
#endif
