.section .rodata

    .align 2
card_palette_pal:
    dc.w    0x0000, 0x000E, 0x00E0, 0x00EE, 0x0E00, 0x0E0E, 0x0EE0, 0x0EEE
    dc.w    0x0CCC, 0x0AAA, 0x0888, 0x0666, 0x0444, 0x0222, 0x000C, 0x000A

    .align 2
card_palette:
    dc.w    0, 16
    dc.l    card_palette_pal

    .align 2
card_animation0_frame0_sprite0_tileset_tiles:
    dc.w    0x0000, 0x0000, 0x008A, 0x8800, 0x00A4, 0xA800, 0x008A, 0x4A00
    dc.w    0x00A4, 0xA800, 0x008A, 0x4A00, 0x0088, 0xA800, 0x0000, 0x0000

    .align 2
card_animation0_frame0_sprite0_tileset:
    dc.w    0
    dc.w    1
    dc.l    card_animation0_frame0_sprite0_tileset_tiles

    .align 2
card_animation0_frame0_sprite0:
    dc.w    0
    dc.w    0
    dc.w    0
    dc.w    0
    dc.l    card_animation0_frame0_sprite0_tileset


    .align 2
card_animation0_frame0_sprites:
    dc.l    card_animation0_frame0_sprite0

    .align 2
card_animation0_frame0:
    dc.w    1
    dc.l    card_animation0_frame0_sprites
    dc.w    0
    dc.l    0
    dc.w    8
    dc.w    8
    dc.w    0


    .align 2
card_animation0_frames:
    dc.l    card_animation0_frame0

    .align 2
card_animation0_sequence:
    dc.w    0x00CB

    .align 2
card_animation0:
    dc.w    1
    dc.l    card_animation0_frames
    dc.w    1
    dc.l    card_animation0_sequence
    dc.w    0


    .align 2
card_animations:
    dc.l    card_animation0

    .align 2
    .global card
card:
    dc.l    card_palette
    dc.w    1
    dc.l    card_animations

