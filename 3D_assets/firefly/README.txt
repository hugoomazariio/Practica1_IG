LUCIERNAGA (FIREFLY) - modelo 3D procedural
============================================

MALLA
  firefly.obj / firefly.mtl   Malla con UVs y normales (27.100 vertices, ~25.600 quads / ~50.500 tris)
                              Y arriba, cabeza hacia +X, longitud ~10.5 unidades (escala arbitraria).
                              Grupos separados: abdomen, elytra_L/R, pronotum, head, thorax, eye_L/R,
                              leg_0..5, antenna_L/R, wing_L/R (cada ala tiene capa top y bot).
                              2 materiales: firefly_body y firefly_wings (alas translucidas).
  firefly.mdl                 MDL en TEXTO, formato Warcraft III (v800), eje Z arriba, 1 hueso, 1 secuencia "Stand".
                              Usa las texturas .tga de 512 px (albedo, alas con alfa, emisiva aditiva).
  source/                     Para Source Engine (GMod, etc.): firefly.smd + firefly.qc + .vmt.
                              El .mdl binario de Source hay que compilarlo con studiomdl.
                              Las texturas hay que pasarlas a .vtf (VTFEdit).

TEXTURAS (2048x2048, un unico atlas UV compartido por toda la malla) - carpeta textures/
  firefly_albedo.png            Color base
  firefly_normal.png            Normal map OpenGL (Y+)  -> Blender, Unity, Godot, glTF
  firefly_normal_dx.png         Normal map DirectX (Y-) -> Unreal, Source
  firefly_roughness.png         Rugosidad
  firefly_metallic.png          Metalico (todo 0)
  firefly_ao.png                AO aproximado a partir de la cavidad del relieve (no es un bake por raytracing)
  firefly_height.png            Mapa de altura
  firefly_emissive.png          Emision (organo luminoso verde-amarillo)
  firefly_emissive_mask.png     Mascara de emision en gris (Source $selfillummask)
  firefly_opacity.png           Opacidad (solo importa para las alas)
  firefly_ORM.png               AO (R) / Rugosidad (G) / Metalico (B) empaquetado
  firefly_uv_layout.png         Mapa UV (wireframe por partes, con los recuadros del atlas)
  firefly_uv_overlay_on_albedo.png  Mapa UV sobre el albedo
  *_512.tga                     Versiones 512 px para el MDL

NOTAS
  - El MDL (texto) y los archivos de Source NO se han probado en el programa de destino.
  - Si tu .mdl es de otro motor/programa distinto, dime cual y lo adapto.
  - Si el visor solo lee map_Bump como bump y no como normal, asigna firefly_normal.png a mano como normal map.
