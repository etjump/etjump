gfx/misc/portal_blueShader
{
	surfaceparm nolightmap
	surfaceparm trans
	//surfaceparm nonsolid
	surfaceparm nodamage
	surfaceparm nomarks
	nomipmaps
	{
		clampMap gfx/misc/teleporter_swirl4.tga
		blendFunc ADD
		tcMod rotate 45
		rgbGen wave sin 0.8 .15 .2 0.05
	}
	{
		clampMap gfx/misc/teleporter_swirl4.tga
		blendFunc ADD
		tcMod rotate 45
		//rgbGen wave sin 0.8 .15 .2 0.05
		rgbGen vertex
	}
	{
		clampMap gfx/misc/teleporter_swirl1.tga
		blendFunc ADD
		tcMod rotate 180
		//rgbGen vertex
          	tcMod stretch sin 0.9 0.1 0 .5
	}
}

gfx/misc/portal_redShader
{
	surfaceparm nolightmap
	surfaceparm trans
	//surfaceparm nonsolid
	surfaceparm nodamage
	surfaceparm nomarks
	nomipmaps
	{
		clampMap gfx/misc/teleporter_swirl4_red.tga
		blendFunc ADD
		tcMod rotate -45
		rgbGen wave sin 0.8 .15 .2 0.05
	}
	{
		clampMap gfx/misc/teleporter_swirl4_red.tga
		blendFunc ADD
		tcMod rotate -45
		//rgbGen wave sin 0.8 .15 .2 0.05
		rgbGen vertex
	}
	{
		clampMap gfx/misc/teleporter_swirl1_red.tga
		blendFunc ADD
		tcMod rotate -180
		//rgbGen vertex
          	tcMod stretch sin 0.9 0.1 0 .5
	}
}

