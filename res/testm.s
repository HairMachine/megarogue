.section .rodata

    .align 2
testm_palette_pal:
    dc.w    0x0000, 0x000E, 0x00E0, 0x00EE, 0x0E00, 0x0E0E, 0x0EE0, 0x0EEE
    dc.w    0x0CCC, 0x0AAA, 0x0888, 0x0666, 0x0444, 0x0222, 0x000C, 0x000A

    .align 2
testm_palette:
    dc.w    0, 16
    dc.l    testm_palette_pal

    .align 2
testm_animation0_frame0_sprite0_tileset_tiles:
    dc.w    0x0000, 0x0000, 0x00FE, 0xEF00, 0x0FFE, 0xEFF0, 0x00F7, 0x9F00
    dc.w    0x0FEE, 0xEEF0, 0x00E7, 0x9E00, 0x0FEE, 0xEEF0, 0x0000, 0x0000

    .align 2
testm_animation0_frame0_sprite0_tileset:
    dc.w    0
    dc.w    1
    dc.l    testm_animation0_frame0_sprite0_tileset_tiles

    .align 2
testm_animation0_frame0_sprite0:
    dc.w    0
    dc.w    0
    dc.w    0
    dc.w    0
    dc.l    testm_animation0_frame0_sprite0_tileset


    .align 2
testm_animation0_frame0_sprites:
    dc.l    testm_animation0_frame0_sprite0

    .align 2
testm_animation0_frame0:
    dc.w    1
    dc.l    testm_animation0_frame0_sprites
    dc.w    0
    dc.l    0
    dc.w    8
    dc.w    8
    dc.w    0


    .align 2
testm_animation0_frames:
    dc.l    testm_animation0_frame0

    .align 2
testm_animation0_sequence:
    dc.w    0x008B

    .align 2
testm_animation0:
    dc.w    1
    dc.l    testm_animation0_frames
    dc.w    1
    dc.l    testm_animation0_sequence
    dc.w    0


    .align 2
testm_animations:
    dc.l    testm_animation0

    .align 2
    .global testm
testm:
    dc.l    testm_palette
    dc.w    1
    dc.l    testm_animations

