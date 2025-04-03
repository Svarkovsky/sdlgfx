#!/usr/bin/env python3

import argparse
import re

def parse_bdf(file_path):
    glyphs = {}
    current_glyph = None
    bitmap = []
    
    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line.startswith('STARTCHAR'):
                current_glyph = {}
            elif line.startswith('ENCODING'):
                current_glyph['codepoint'] = int(line.split()[1])
            elif line.startswith('BITMAP'):
                bitmap = []
            elif line.startswith('ENDCHAR'):
                if len(bitmap) > 0:
                    glyph_bitmap = [0] * 16
                    for i, row in enumerate(bitmap[:16]):
                        val = int(row[:2], 16)  # Берем первые 8 бит
                        glyph_bitmap[i] = val
                    glyphs[current_glyph['codepoint']] = glyph_bitmap
                current_glyph = None
                bitmap = []
            elif current_glyph and re.match(r'^[0-9A-F]+$', line):
                bitmap.append(line)
    
    return glyphs

def generate_header(glyphs, output_file):
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write('#ifndef SDLFONT_UNICODE_H\n')
        f.write('#define SDLFONT_UNICODE_H\n\n')
        f.write('#include <stdint.h>\n\n')
        
        f.write('typedef struct { uint32_t codepoint; uint8_t bitmap[16]; } Glyph;\n\n')
        
        f.write('static const Glyph font_data[] = {\n')
        
        filtered_glyphs = {k: v for k, v in glyphs.items() if (32 <= k <= 126) or (0x0400 <= k <= 0x04FF)}
        glyph_list = list(filtered_glyphs.items())
        
        for i, (codepoint, bitmap) in enumerate(glyph_list):
            f.write(f'    {{0x{codepoint:04X}, {{')
            f.write(', '.join(f'0x{row:02X}' for row in bitmap))
            if i < len(glyph_list) - 1:
                f.write('}},\n')
            else:
                f.write('}}\n')
        
        f.write('};\n\n')
        f.write('static const int font_data_size = sizeof(font_data) / sizeof(font_data[0]);\n')
        f.write('#endif // SDLFONT_UNICODE_H\n')

def main():
    parser = argparse.ArgumentParser(description='Convert BDF font to sdlfont-compatible header.')
    parser.add_argument('-in', '--input', required=True, help='Input BDF file')
    parser.add_argument('-out', '--output', required=True, help='Output header file')
    args = parser.parse_args()

    glyphs = parse_bdf(args.input)
    generate_header(glyphs, args.output)
    print(f"Generated {args.output} with {len(glyphs)} glyphs (filtered to ASCII + Cyrillic).")

if __name__ == '__main__':
    main()
