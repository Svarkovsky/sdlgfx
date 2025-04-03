Вот небольшой мануал на английском языке с упрощённой и понятной логикой для новичков. Я постараюсь объяснить всё так, чтобы человек, впервые работающий с этим кодом, понял, как шрифты выбираются в программе, с примерами.

---

### A Simple Guide to How Fonts Work in Our Program

This guide explains how our program picks fonts to display text (like letters "A" or "Привет") based on two things: a special file called PSF and a compilation flag called `-DUSE_UNICODE`. Think of it as a set of rules the program follows to decide where to get the shapes of letters from!

#### Key Terms
- **PSF File**: A font file (like `Unifont-APL8x16-16.0.02.psf`) that contains shapes for up to 256 characters (numbers 0 to 255). It might include "A", "B", "!", etc., but usually not Cyrillic (like "П").
- **`-DUSE_UNICODE`**: A flag you add when compiling the program (e.g., `gcc ... -DUSE_UNICODE`). It tells the program to use a file called `sdlfont_unicode.h`, which has shapes for lots of characters, including "A" and "П".
- **Built-in ASCII**: A basic set of character shapes (like "A", "!", " ") hard-coded in the program using a `switch` statement. It only covers 32 to 126 (basic English letters and symbols).
- **Codepoint (c)**: Every character has a number (e.g., "A" = 65, "П" = 1040). We use this number to find its shape.

#### How It Works: The Rules
The program checks these rules step-by-step to draw each character. Here’s what happens in different situations:

---

##### 1. When You Compile with `-DUSE_UNICODE` and PSF File is NOT Found
- **What Happens**: If the PSF file (e.g., `Unifont-APL8x16-16.0.02.psf`) isn’t found or fails to load, the program skips it completely. It looks at `sdlfont_unicode.h` instead.
- **Details**: 
  - The check `c <= 255 && psf_font_data` doesn’t work because `psf_font_data` (the PSF data) is empty.
  - Every character—English letters like "A" (65) and Cyrillic like "П" (1040)—comes from `sdlfont_unicode.h`.
  - The built-in `switch` (hard-coded shapes) isn’t used at all.
- **Example**: 
  - You write `"Hello, Привет!"`.
  - Result: "Hello" (ASCII) and "Привет" (Cyrillic) both come from `sdlfont_unicode.h`. Everything looks correct!

---

##### 2. When You Compile with `-DUSE_UNICODE` and PSF File is Loaded
- **What Happens**: If the PSF file loads successfully, it’s used for some characters, but `sdlfont_unicode.h` still helps for others.
- **Details**:
  - For characters with numbers **0 to 255** (e.g., "A" = 65, "!" = 33), the program uses the PSF file.
  - For characters with numbers **above 255** (e.g., "П" = 1040, Cyrillic), it uses `sdlfont_unicode.h`.
- **Example**: 
  - You write `"Hello, Привет!"`.
  - Result: "Hello" comes from the PSF file (since 72, 101, etc., are below 255), and "Привет" comes from `sdlfont_unicode.h` (since 1040, 1088, etc., are above 255).

---

##### 3. When You Compile WITHOUT `-DUSE_UNICODE`
- **What Happens**: Without the `-DUSE_UNICODE` flag, the program ignores `sdlfont_unicode.h` completely and relies on the PSF file or built-in shapes.
- **Details**:
  - If the PSF file is loaded, it’s used for characters **0 to 255**.
  - If the PSF file isn’t found, the program uses the built-in ASCII (`switch`) for characters **32 to 126**, and anything else (like "П") turns into a "?".
- **Example**: 
  - You write `"Hello, Привет!"`.
  - If PSF is loaded: "Hello" comes from PSF, "Привет" becomes "??????" (Cyrillic isn’t supported).
  - If PSF isn’t found: "Hello" comes from `switch`, "Привет" becomes "??????".

---

### Quick Summary Table
| Situation                  | PSF Loaded? | ASCII (0–255) Source       | Cyrillic (>255) Source    |
|----------------------------|-------------|----------------------------|---------------------------|
| `-DUSE_UNICODE`, PSF not found | No          | `sdlfont_unicode.h`        | `sdlfont_unicode.h`       |
| `-DUSE_UNICODE`, PSF loaded   | Yes         | PSF file                  | `sdlfont_unicode.h`       |
| No `-DUSE_UNICODE`, PSF loaded| Yes         | PSF file                  | "?" (not supported)       |
| No `-DUSE_UNICODE`, no PSF    | No          | Built-in ASCII (`switch`) | "?" (not supported)       |

---

### How to Test It
1. **With `-DUSE_UNICODE` and no PSF:**
   - Compile: `gcc -o myprogram myprogram.c sdlfont.c sdlgfx.c -lSDL2 -lm -DUSE_UNICODE`
   - Remove or rename `Unifont-APL8x16-16.0.02.psf`.
   - Run: `./myprogram`
   - See: `"Hello, Привет!"` fully displayed from `sdlfont_unicode.h`.

2. **With `-DUSE_UNICODE` and PSF:**
   - Put `Unifont-APL8x16-16.0.02.psf` in the folder.
   - Compile and run the same way.
   - See: `"Hello"` from PSF, `"Привет"` from `sdlfont_unicode.h`.

3. **Without `-DUSE_UNICODE`:**
   - Compile: `gcc -o myprogram myprogram.c sdlfont.c sdlgfx.c -lSDL2 -lm`
   - Try with and without PSF.
   - See: `"Hello"` from PSF or `switch`, `"Привет"` as `??????`.

---

### Why It’s Cool
- **Flexible**: You get the best of both worlds—PSF for simple stuff, Unicode for fancy letters like Cyrillic.
- **Clear Rules**: Easy to predict where your letters come from based on what you’ve got (PSF or Unicode).

If you’re new and something still непонятно (oops, unclear!), just ask—I’ll explain simpler!