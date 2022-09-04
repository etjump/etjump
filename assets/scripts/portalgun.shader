
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