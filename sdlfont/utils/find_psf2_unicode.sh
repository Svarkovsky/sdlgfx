#!/bin/bash
dir="."  # Укажите вашу директорию здесь
found=0

echo "Поиск PSF2 шрифтов с Юникодом в: $dir"
echo "----------------------------------------"

while IFS= read -r -d '' file; do
    echo -n "Проверка: $file... "

    # Проверка сигнатуры PSF2 (0x72 0xb5 0x4a 0x86)
    header=$(head -c 4 "$file" | hexdump -n 4 -e '4/1 "%02x "' 2>/dev/null)
    if [[ "$header" != "72 b5 4a 86 " ]]; then
        echo "не PSF2"
        continue
    fi

    # Проверка флага Unicode (смещение 12, 4 байта, поле flags, бит 0x01)
    flags=$(head -c 16 "$file" | tail -c 4 | hexdump -n 4 -e '/4 "%u"' 2>/dev/null)
    has_unicode=$((flags & 0x01))
    if [[ "$has_unicode" != "1" ]]; then
        echo "PSF2 без Юникода"
        continue
    fi

    # Если дошли сюда — найден PSF2 с Юникодом
    echo "НАЙДЕНО: PSF2 с Юникодом!"
    echo ">>> $file"
    echo "----------------------------------------"
    found=$((found + 1))
done < <(find "$dir" -type f -print0)

echo "Поиск завершён."
if [[ "$found" -eq 0 ]]; then
    echo "PSF2 шрифтов с Юникодом не найдено."
else
    echo "Всего найдено: $found файлов"
fi
