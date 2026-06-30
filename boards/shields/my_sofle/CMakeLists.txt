# =============================================================================
# CMakeLists.txt — rejestracja plików źródłowych C w systemie budowania ZMK
# =============================================================================
#
# TEN PLIK JEST ODCZYTYWANY PRZEZ SYSTEM BUDOWANIA ZEPHYR/ZMK.
# Każdy shield (np. my_sofle) może posiadać swój CMakeLists.txt.
# Zephyr automatycznie go wczytuje i kompiluje wymienione tu pliki .c
# razem z resztą firmware ZMK.
#
# JAK TO DZIAŁA:
#   1. Sprawdzamy, czy CONFIG_ZMK_DISPLAY jest włączony w Kconfig (.conf).
#   2. Jeśli tak, dodajemy ścieżkę do nagłówków ZMK (potrzebne do #include).
#   3. Rejestrujemy nasz plik custom_status_screen.c jako źródło do kompilacji.
#
# DLACZEGO TO POTRZEBNE:
#   Normalnie ZMK używa wbudowanego ekranu statusu (status_screen.c w kodzie ZMK).
#   Ale jeśli w Kconfig ustawimy CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y,
#   ZMK oczekuje, że MY dostarczymy funkcję zmk_display_status_screen().
#   Ten plik mówi kompilatorowi, gdzie znaleźć naszą implementację.
#
# DODAWANIE NOWYCH PLIKÓW:
#   Jeśli stworzysz np. widgets/bongo_cat.c, dodaj poniżej:
#     zephyr_library_sources(widgets/bongo_cat.c)
# =============================================================================

if(CONFIG_ZMK_DISPLAY)
  # Dodaj ścieżkę do nagłówków ZMK (potrzebne do #include <zmk/...>)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)

  # Nasz niestandardowy ekran statusu (ZAKOMENTOWANY)
  # zephyr_library_sources(custom_status_screen.c)
endif()
