.section .rodata

    .align 2
stairs_palette_pal:
    dc.w    0x0000, 0x000E, 0x00E0, 0x00EE, 0x0E00, 0x0E0E, 0x0EE0, 0x0EEE
    dc.w    0x0CCC, 0x0AAA, 0x0888, 0x0666, 0x0444, 0x0222, 0x000C, 0x000A

    .align 2
stairs_palette:
    dc.w    0, 16
    dc.l    stairs_palette_pal

    .align 2
stairs_animation0_frame0_sprite0_tileset_tiles:
    dc.w    0x0000, 0x0000, 0x000A, 0xA000, 0x0000, 0x0000, 0x0099, 0x9900
    dc.w    0x0000, 0x0000, 0x0888, 0x8880, 0x0000, 0x0000, 0x7777, 0x7777

    .align 2
stairs_animation0_frame0_sprite0_tileset:
    dc.w    0
    dc.w    1
    dc.l    stairs_animation0_frame0_sprite0_tileset_tiles

    .align 2
stairs_animation0_frame0_sprite0:
    dc.w    0
    dc.w    0
    dc.w    0
    dc.w    0
    dc.l    stairs_animation0_frame0_sprite0_tileset


    .align 2
stairs_animation0_frame0_sprites:
    dc.l    stairs_animation0_frame0_sprite0

    .align 2
stairs_animation0_frame0:
    dc.w    1
    dc.l    stairs_animation0_frame0_sprites
    dc.w    0
    dc.l    0
    dc.w    8
    dc.w    8
    dc.w    0


    .align 2
stairs_animation0_frames:
    dc.l    stairs_animation0_frame0

    .align 2
stairs_animation0_sequence:
    dc.w    0x009B

    .align 2
stairs_animation0:
    dc.w    1
    dc.l    stairs_animation0_frames
    dc.w    1
    dc.l    stairs_animation0_sequence
    dc.w    0


    .align 2
stairs_animations:
    dc.l    stairs_animation0

    .align 2
    .global stairs
stairs:
    dc.l    stairs_palette
    dc.w    1
    dc.l    stairs_animations

