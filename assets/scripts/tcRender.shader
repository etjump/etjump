// unlagged's bbox shaders adapated
tcRenderShader
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
		rgbGen vertex
	}
}

tcRenderShader_nocull
{
	nopicmip
	cull none
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
		rgbGen vertex
	}
}

tcRenderShader1r
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.2 0 0 )
	}
}

tcRenderShader1g
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0 0.2 0 )
	}
}

tcRenderShader1b
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0 0 0.2 )
	}
}

tcRenderShader1c
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0 0.2 0.2 )
	}
}

tcRenderShader1m
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.2 0 0.2 )
	}
}

tcRenderShader1y
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.2 0.2 0 )
	}
}

tcRenderShader2r
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.05 0 0 )
	}
}

tcRenderShader2g
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0 0.05 0 )
	}
}

tcRenderShader2b
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0 0 0.05 )
	}
}

tcRenderShader2c
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0 0.05 0.05 )
	}
}

tcRenderShader2m
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.05 0 0.05 )
	}
}

tcRenderShader2y
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.05 0.05 0 )
	}
}


tcRenderShader3r
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader_tr.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.5 0 0 )
	}
}

tcRenderShader3g
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader_tr.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0 0.5 0 )
	}
}

tcRenderShader3b
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader_tr.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0 0 0.5 )
	}
}

tcRenderShader3c
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader_tr.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0 0.5 0.5 )
	}
}

tcRenderShader3m
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader_tr.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.5 0 0.5 )
	}
}

tcRenderShader3y
{
	polygonOffset
	nopicmip
	{
		map gfx/misc/tcRenderShader_tr.tga
		blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.5 0.5 0 )
	}
}
