/*
 * =============================================================================
 * custom_status_screen.c — Niestandardowy ekran OLED dla klawiatury My Sofle
 * =============================================================================
 *
 * CZYM JEST TEN PLIK?
 *   Ten plik definiuje GŁÓWNĄ FUNKCJĘ wyświetlacza:
 *     lv_obj_t *zmk_display_status_screen()
 *
 *   ZMK wywołuje tę funkcję JEDEN RAZ przy starcie firmware.
 *   Zwraca ona obiekt ekranu LVGL, który jest potem rysowany na OLED-zie.
 *
 * CO TO JEST LVGL?
 *   LVGL (Light and Versatile Graphics Library) to lekka biblioteka graficzna
 *   używana przez Zephyr RTOS i ZMK do rysowania na ekranach.
 *   Kluczowe pojęcia:
 *     - lv_obj_t     — bazowy obiekt graficzny (kontener, etykieta, itd.)
 *     - lv_label     — widget tekstowy (wyświetla napis)
 *     - lv_obj_align — ustawia pozycję obiektu (TOP_LEFT, CENTER, itd.)
 *     - lv_style     — styl graficzny (czcionka, kolor, marginesy)
 *
 * JAK DZIAŁA UKŁAD EKRANU?
 *   Twój ekran SH1106 jest zamontowany PIONOWO (portrait):
 *     - Szerokość:  64 piksele
 *     - Wysokość: 128 pikseli
 *
 *   Poniżej rozmieszczamy widgety od góry do dołu:
 *
 *     ┌──────────────────┐  y=0
 *     │  BAT%  ·  USB/BT │  <- Wiersz statusu (bateria + wyjście)
 *     │──────────────────│  y~16
 *     │                  │
 *     │   ╔════════════╗ │
 *     │   ║  WARSTWA   ║ │  <- Nazwa aktywnej warstwy (duża czcionka)
 *     │   ╚════════════╝ │
 *     │                  │  y~80
 *     │──────────────────│
 *     │     42 WPM       │  <- Prędkość pisania (Words Per Minute)
 *     └──────────────────┘  y=128
 *
 * WBUDOWANE WIDGETY ZMK:
 *   ZMK dostarcza gotowe widgety, które reagują na zdarzenia firmware:
 *     - zmk_widget_battery_status  — pokazuje poziom baterii
 *     - zmk_widget_output_status   — pokazuje USB / BT1 / BT2 ...
 *     - zmk_widget_layer_status    — pokazuje nazwę aktywnej warstwy
 *     - zmk_widget_wpm_status      — pokazuje prędkość pisania (WPM)
 *     - zmk_widget_peripheral_status — (tylko prawa strona split) status połączenia
 *
 *   Każdy widget ma trzy funkcje:
 *     _init(widget, parent)  — tworzy widget i podpina go do ekranu
 *     _obj(widget)           — zwraca wskaźnik do obiektu LVGL (do pozycjonowania)
 *
 * JAK MODYFIKOWAĆ?
 *   1. Aby ZMIENIĆ POZYCJĘ widgetu, edytuj wywołania lv_obj_align().
 *      Dostępne stałe: LV_ALIGN_TOP_LEFT, LV_ALIGN_TOP_MID, LV_ALIGN_TOP_RIGHT,
 *                       LV_ALIGN_CENTER, LV_ALIGN_BOTTOM_LEFT, itd.
 *      Ostatnie dwa argumenty to przesunięcie (x, y) w pikselach.
 *
 *   2. Aby ZMIENIĆ CZCIONKĘ, użyj lv_obj_set_style_text_font().
 *      Dostępne czcionki Montserrat: lv_font_montserrat_12, _14, _16, _18, _20...
 *      (muszą być włączone w Kconfig: CONFIG_LV_FONT_MONTSERRAT_XX=y)
 *
 *   3. Aby DODAĆ WŁASNY TEKST statyczny:
 *        lv_obj_t *label = lv_label_create(screen);
 *        lv_label_set_text(label, "Hej!");
 *        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -5);
 *
 *   4. Aby DODAĆ LINIĘ SEPARATORA:
 *        // patrz sekcja "Linie dekoracyjne" w kodzie poniżej
 * =============================================================================
 */

#include <zmk/display/widgets/output_status.h>
#include <zmk/display/widgets/peripheral_status.h>
#include <zmk/display/widgets/battery_status.h>
#include <zmk/display/widgets/layer_status.h>
#include <zmk/display/widgets/wpm_status.h>
#include <zmk/display/status_screen.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* ─── Deklaracje instancji widgetów ───────────────────────────────────────── */

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY_STATUS)
static struct zmk_widget_battery_status battery_status_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_OUTPUT_STATUS)
static struct zmk_widget_output_status output_status_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
static struct zmk_widget_peripheral_status peripheral_status_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER_STATUS)
static struct zmk_widget_layer_status layer_status_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
static struct zmk_widget_wpm_status wpm_status_widget;
#endif

/* =============================================================================
 * GŁÓWNA FUNKCJA EKRANU
 * =============================================================================
 * Ta funkcja jest wywoływana przez ZMK przy starcie.
 * Tworzy ekran LVGL i rozmieszcza na nim widgety.
 *
 * WAŻNE: Funkcja MUSI mieć dokładnie tę sygnaturę:
 *   lv_obj_t *zmk_display_status_screen()
 * ZMK szuka jej po nazwie podczas linkowania.
 *
 * UKŁAD PIONOWY (64x128px):
 *   y=0   → Status (USB/BT + bateria)
 *   y=40  → Nazwa warstwy (centrum)
 *   y=110 → WPM
 * =============================================================================
 */
lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    /* ── Górna sekcja: Status połączenia i baterii ────────────────────────── */

#if IS_ENABLED(CONFIG_ZMK_WIDGET_OUTPUT_STATUS)
    zmk_widget_output_status_init(&output_status_widget, screen);
    lv_obj_align(zmk_widget_output_status_obj(&output_status_widget),
                 LV_ALIGN_TOP_LEFT, 0, 2);
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY_STATUS)
    zmk_widget_battery_status_init(&battery_status_widget, screen);
    lv_obj_align(zmk_widget_battery_status_obj(&battery_status_widget),
                 LV_ALIGN_TOP_RIGHT, 0, 2);
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
    zmk_widget_peripheral_status_init(&peripheral_status_widget, screen);
    lv_obj_align(zmk_widget_peripheral_status_obj(&peripheral_status_widget),
                 LV_ALIGN_TOP_LEFT, 0, 2);
#endif

    /* ── Środkowa sekcja: Nazwa aktywnej warstwy ─────────────────────────── */

#if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER_STATUS)
    zmk_widget_layer_status_init(&layer_status_widget, screen);
    lv_obj_align(zmk_widget_layer_status_obj(&layer_status_widget),
                 LV_ALIGN_CENTER, 0, -10);
#endif

    /* ── Dolna sekcja: WPM ────────────────────────────────────────────────── */

#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
    zmk_widget_wpm_status_init(&wpm_status_widget, screen);
    lv_obj_align(zmk_widget_wpm_status_obj(&wpm_status_widget),
                 LV_ALIGN_BOTTOM_MID, 0, -5);
#endif

    return screen;
}

