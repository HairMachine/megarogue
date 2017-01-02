.section .rodata

    .align 2
template_dude_palette_pal:
    dc.w    0x0000, 0x000E, 0x00E0, 0x00EE, 0x0E00, 0x0E0E, 0x0EE0, 0x0EEE
    dc.w    0x0CCC, 0x0AAA, 0x0888, 0x0666, 0x0444, 0x0222, 0x000C, 0x000A

    .align 2
template_dude_palette:
    dc.w    0, 16
    dc.l    template_dude_palette_pal

    .align 2
template_dude_animation0_frame0_sprite0_tileset_tiles:
    dc.w    0x0000, 0x0000, 0x0000, 0x0101, 0x0000, 0x1111, 0x0001, 0x1313
    dc.w    0x0001, 0x1333, 0x0000, 0x1373, 0x0000, 0x3343, 0x0000, 0x3333
    dc.w    0x0000, 0x0333, 0x0000, 0x6633, 0x0006, 0x6666, 0x0036, 0x0666
    dc.w    0x0030, 0x0066, 0x0000, 0x0444, 0x0000, 0x0440, 0x0000, 0x4440
    dc.w    0x0000, 0x0000, 0x1010, 0x0000, 0x1111, 0x0000, 0x1111, 0x1000
    dc.w    0x3331, 0x1000, 0x3731, 0x0000, 0x3433, 0x0000, 0x3333, 0x0000
    dc.w    0x3330, 0x0000, 0x3366, 0x0000, 0x6666, 0x6000, 0x6660, 0x6300
    dc.w    0x6600, 0x0300, 0x4440, 0x0000, 0x0440, 0x0000, 0x0444, 0x0000

    .align 2
template_dude_animation0_frame0_sprite0_tileset:
    dc.w    0
    dc.w    4
    dc.l    template_dude_animation0_frame0_sprite0_tileset_tiles

    .align 2
template_dude_animation0_frame0_sprite0:
    dc.w    0
    dc.w    1280
    dc.w    0
    dc.w    0
    dc.l    template_dude_animation0_frame0_sprite0_tileset


    .align 2
template_dude_animation0_frame0_sprites:
    dc.l    template_dude_animation0_frame0_sprite0

    .align 2
template_dude_animation0_frame0:
    dc.w    1
    dc.l    template_dude_animation0_frame0_sprites
    dc.w    0
    dc.l    0
    dc.w    16
    dc.w    16
    dc.w    0


    .align 2
template_dude_animation0_frames:
    dc.l    template_dude_animation0_frame0

    .align 2
template_dude_animation0_sequence:
    dc.w    0x005B

    .align 2
template_dude_animation0:
    dc.w    1
    dc.l    template_dude_animation0_frames
    dc.w    1
    dc.l    template_dude_animation0_sequence
    dc.w    0


    .align 2
template_dude_animations:
    dc.l    template_dude_animation0

    .align 2
    .global template_dude
template_dude:
    dc.l    template_dude_palette
    dc.w    1
    dc.l    template_dude_animations

