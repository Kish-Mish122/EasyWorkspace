# EasyOffice 0.1 (прототип)

Лёгкий офисный пакет на **C++20 + Qt 6 (Widgets)**: три независимых приложения и общая библиотека.

| Приложение | Что умеет | Свой формат |
|---|---|---|
| **EasyWrite** | форматирование (шрифт, размер, Ж/К/Ч, цвет, выделение), выравнивание, списки, таблицы, картинки, поиск и замена, счётчик слов; экспорт в PDF / ODT / HTML; открытие .html и .txt | `.ezw` |
| **EasySheets** | сетка 500×52, формулы (`+ - * / ^`, скобки, `A1`, диапазоны), функции `SUM AVG MIN MAX COUNT` (и `СУММ СРЗНАЧ МИН МАКС СЧЁТ`), защита от циклов, строка формул, жирный и заливка, копирование/вставка, статистика выделения, диаграммы (столбцы/линия/круг → PNG), импорт/экспорт CSV | `.ezx` |
| **EasySlides** | слайды с миниатюрами, текст/прямоугольник/эллипс, перетаскивание и изменение размера мышью, цвета, шрифт, фон, 4 темы оформления, показ на весь экран (F5), экспорт в PDF | `.ezp` |

Все форматы — обычный JSON, их легко смотреть и отлаживать.

## Структура

```
EasyOffice/
├── CMakeLists.txt
├── core/          # EasyCore: тема, формулы, модель таблицы
├── word/          # EasyWord
├── excel/         # EasyExcel (+ диаграммы)
├── powerpoint/    # EasyPowerPoint
└── installer/     # EasyOffice.iss (Inno Setup)
```

## Сборка

Нужны: компилятор C++20 (GCC 11+, Clang 14+, MSVC 2022 или MinGW), CMake ≥ 3.21, Qt ≥ 6.2.

**Linux** (Ubuntu/Debian: `sudo apt install build-essential cmake qt6-base-dev`):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/word/EasyWord
./build/excel/EasyExcel
./build/powerpoint/EasyPowerPoint
```

**Windows** (Qt из онлайн-установщика, например MSVC 2022 64-bit):

```bat
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.7.0\msvc2019_64
cmake --build build
```

Можно просто открыть папку в Qt Creator (File → Open File or Project → CMakeLists.txt).

## Windows-установщик

1. Соберите Release и создайте папку `installer\deploy`.
2. Скопируйте туда три `.exe` и для каждого запустите (из «Qt 6.x for MSVC» командной строки):
   ```bat
   windeployqt --release --no-translations installer\deploy\EasyWord.exe
   windeployqt --release --no-translations installer\deploy\EasyExcel.exe
   windeployqt --release --no-translations installer\deploy\EasyPowerPoint.exe
   ```
3. Откройте `installer\EasyOffice.iss` в Inno Setup и нажмите Compile — получится `installer\output\EasyOffice-Setup-0.1.0.exe`.

Установщик: выбор компонентов (Word / Excel / PowerPoint), ярлыки в меню «Пуск» и на рабочем столе, ассоциации `.ezw`, `.ezx`, `.ezp`.

## Чего пока нет (план)

- Импорт/экспорт DOCX, XLSX, PPTX (сейчас: PDF, ODT, HTML, CSV и собственные форматы)
- Отмена/повтор в EasyExcel и EasyPowerPoint (в EasyWord есть)
- Иконки приложений, картинки и переходы в презентациях, больше функций в формулах
- Тёмная тема интерфейса, переводы
