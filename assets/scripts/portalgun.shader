
models/weapons2/portalgun/portalgun_main
{
    cull none
    {
        map textures/effects/envmap_slate.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
        tcGen environment
        alphaGen normalzfade 1.0 -200 200
        depthWrite
    }
    {
        map models/weapons2/portalgun/portalgun_main.tga
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
        alphaGen normalzfade 1.0 -200 200
        depthWrite
    }
}

models/weapons2/portalgun/portalgun_barrel1
{
    cull none
    {
        map textures/effects/envmap_slate.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
        tcGen environment
        alphaGen normalzfade 1.0 -200 200
        depthWrite
    }
    {
        map models/weapons2/portalgun/portalgun_barrel1.tga
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
        alphaGen normalzfade 1.0 -200 200
        depthWrite
    }
}

models/weapons2/portalgun/portalgun_barrel2
{
    cull none
    {
        map textures/effects/envmap_slate.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
        tcGen environment
        alphaGen normalzfade 1.0 -200 200
        depthWrite
    }
    {
        map models/weapons2/portalgun/portalgun_barrel2.tga
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
        alphaGen normalzfade 1.0 -200 200
        depthWrite
    }
}

models/weapons2/portalgun/electricity
{
    cull none
    {
        map models/weapons2/portalgun/zap_scroll.tga
        blendFunc GL_ONE GL_ONE
        rgbgen wave triangle .8 0.5 0 0.1
        tcMod scroll 0 0.8
        rgbGen const ( 1.0 0.3 0.1 )
    }   
    {
        map models/weapons2/portalgun/zap_scroll.tga
        blendFunc GL_ONE GL_ONE
        rgbgen wave triangle 1 0.2 0 0.3
        tcMod scale  -1 1
        tcMod scroll 0 0.8
        rgbGen const ( 1.0 0.3 0.1 )
    }   
    {
        map models/weapons2/portalgun/zap_scroll2a.tga
        blendFunc GL_ONE GL_ONE
        rgbgen wave triangle 1 0.2 0 0.4
        tcMod scale  -1 1
        tcMod scroll 4 0.8
        rgbGen const ( 1.0 0.3 0.1 )
    }   
    {
        map models/weapons2/portalgun/zap_scroll2a.tga
        blendFunc GL_ONE GL_ONE
        rgbgen wave triangle 1 0.2 0 0.2
        tcMod scroll -1.2 0.8
        rgbGen const ( 1.0 0.3 0.1 )
    }   
    }

models/weapons2/portalgun/portalgun_3rdP
{
    cull none
    {
        map textures/effects/envmap_slate.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
        tcGen environment
        alphaGen normalzfade 1.0 -200 200
        depthWrite
    }
    {
        map models/weapons2/portalgun/portalgun_3rdP.tga
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
        alphaGen normalzfade 1.0 -200 200
        depthWrite
    }
}

gfx/misc/portal_blue
{
	nopicmip
	nomipmaps
	cull none
	{
		clampMap gfx/misc/portal_blue.tga
		blendFunc GL_ONE GL_ONE
		alphaGen vertex
		rgbGen identityLighting
		tcMod rotate 70
		tcMod stretch sin 1 0.05 0.25 0.8
	}
	{
		clampMap gfx/misc/portal_blue.tga
		blendFunc GL_ONE GL_ONE
		alphaGen vertex
		rgbGen identityLighting
		tcMod rotate -100
		tcMod stretch sin 1 0.05 1 0.7
	}
}

gfx/misc/portal_red
{
	nopicmip
	nomipmaps
	cull none
	{
		clampMap gfx/misc/portal_red.tga
		blendFunc GL_ONE GL_ONE
		alphaGen vertex
		rgbGen identityLighting
		tcMod rotate 70
		tcMod stretch sin 1 0.05 0.25 0.8
	}
	{
		clampMap gfx/misc/portal_red.tga
		blendFunc GL_ONE GL_ONE
		alphaGen vertex
		rgbGen identityLighting
		tcMod rotate -100
		tcMod stretch sin 1 0.05 1 0.7
	}
}

gfx/misc/portal_green
{
	nopicmip
	nomipmaps
	cull none
	{
		clampMap gfx/misc/portal_green.tga
		blendFunc GL_ONE GL_ONE
		alphaGen vertex
		rgbGen identityLighting
		tcMod rotate 70
		tcMod stretch sin 1 0.05 0.25 0.8
	}
	{
		clampMap gfx/misc/portal_green.tga
		blendFunc GL_ONE GL_ONE
		alphaGen vertex
		rgbGen identityLighting
		tcMod rotate -100
		tcMod stretch sin 1 0.05 1 0.7
	}
}

gfx/misc/portal_yellow
{
	nopicmip
	nomipmaps
	cull none
	{
		clampMap gfx/misc/portal_yellow.tga
		blendFunc GL_ONE GL_ONE
		alphaGen vertex
		rgbGen identityLighting
		tcMod rotate 70
		tcMod stretch sin 1 0.05 0.25 0.8
	}
	{
		clampMap gfx/misc/portal_yellow.tga
		blendFunc GL_ONE GL_ONE
		alphaGen vertex
		rgbGen identityLighting
		tcMod rotate -100
		tcMod stretch sin 1 0.05 1 0.7
	}
}