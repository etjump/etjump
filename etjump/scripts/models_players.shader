models/players/multi/acc/backpack/backpack_lieu
{
	cull disable //Rich - Why does this shader have cull disable??
	{
		map models/players/multi/acc/backpack/backpack_lieu.tga
		alphafunc ge128
		rgbGen lightingDiffuse
	}
}

models/players/temperate/axis/ranks/major
{
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/major.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

models/players/temperate/axis/ranks/feldwebel
{
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/sergeant.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

models/players/temperate/axis/ranks/leutnant
{
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/lieutenant.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}


models/players/temperate/axis/ranks/gefreiter
{
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/corporal.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}



models/players/temperate/axis/ranks/oberst
{
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/colonel.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

models/players/temperate/axis/ranks/hauptmann
{
	polygonoffset	
	
	{
		map models/players/temperate/allied/ranks/captain.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}



models/players/temperate/axis/ranks/oberschutze
{
	polygonoffset	
	
	{
		map models/players/temperate/allied/ranks/1stclass.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}



models/players/temperate/allied/ranks/major
{
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/major.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

models/players/temperate/allied/ranks/sergeant
{
	qer_models/players/temperate/allied/ranks/sergeant.tga
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/sergeant.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

models/players/temperate/allied/ranks/lieutenant
{
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/lieutenant.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}


models/players/temperate/allied/ranks/corporal
{
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/corporal.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}



models/players/temperate/allied/ranks/colonel
{
	polygonoffset
	
	{
		map models/players/temperate/allied/ranks/colonel.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}


models/players/temperate/allied/ranks/captain
{
	polygonoffset	
	
	{
		map models/players/temperate/allied/ranks/captain.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}



models/players/temperate/allied/ranks/1stclass
{
	polygonoffset	
	
	{
		map models/players/temperate/allied/ranks/1stclass.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

//****************************************
//
//	Axis body
//
//****************************************
models/players/temperate/axis/engineer/body_engineer
{
	sort 16
	{
		map models/players/temperate/axis/engineer/body_engineer.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/cvops/body_cvops
{
	sort 16
	{
		map models/players/temperate/axis/cvops/body_cvops.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/fieldops/body_fieldops
{
	sort 16
	{
		map models/players/temperate/axis/fieldops/body_fieldops.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/medic/axis_medic
{
	sort 16
	{
		map models/players/temperate/axis/medic/axis_medic.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/soldier/body_soldier
{
	sort 16
	{
		map models/players/temperate/axis/soldier/body_soldier.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

//*****************************************
//
//	Axis gloves
//
//*****************************************
models/players/temperate/axis/body01
{
	sort 16
	{
		map models/players/temperate/axis/body01.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

//******************************************
//	
//	Axis legs
//
//******************************************
models/players/temperate/axis/medic/legs_medic
{
	sort 16
	{
		map models/players/temperate/axis/medic/legs_medic.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/legs01 
{
	sort 16
	{
		map models/players/temperate/axis/legs01.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

//*******************************************
//	
//	Axis backpacks
//
//*******************************************
models/players/temperate/axis/medic/acc/backpack 
{
	sort 16
	{
		map models/players/temperate/axis/medic/acc/backpack.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/medic/acc/backpack2 
{
	sort 16	
	{
		map models/players/temperate/axis/medic/acc/backpack2.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/cvops/acc/backpack 
{
	sort 16	
	{
		map models/players/temperate/axis/cvops/acc/backpack.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/cvops/acc/fieldkit 
{
	sort 16	
	{
		map models/players/temperate/axis/cvops/acc/fieldkit.tga 
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/engineer/acc/backpack 
{
	sort 16	
	{
		map models/players/temperate/axis/engineer/acc/backpack.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/engineer/acc/shovel 
{
	sort 16	
	{
		map models/players/temperate/axis/engineer/acc/shovel.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/fieldops/acc/backpack 
{
	sort 16	
	{
		map models/players/temperate/axis/fieldops/acc/backpack.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/soldier/acc/backpack 
{
	sort 16	
	{
		map models/players/temperate/axis/soldier/acc/backpack.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

//********************************************
//
//	Axis helmets
//
//********************************************
models/players/temperate/axis/medic/helmet 
{
	sort 16
	{
		map models/players/temperate/axis/medic/helmet.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/cvops/cap 
{
	sort 16
	{
		map models/players/temperate/axis/cvops/cap.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/engineer/helmet 
{
	sort 16
	{
		map models/players/temperate/axis/engineer/helmet.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/fieldops/helmet 
{
	sort 16
	{
		map models/players/temperate/axis/fieldops/helmet.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/soldier/helmet 
{
	sort 16
	{
		map models/players/temperate/axis/soldier/helmet.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/axis/inside
{
	sort 16
	{
		map models/players/temperate/axis/inside.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

//*****************************************
//
//	Axis heads
//
//*****************************************
models/players/hud/axis_medic 
{
	sort 16
	{
		map models/players/hud/axis_medic.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/axis_cvops 
{
	sort 16	
	{
		map models/players/hud/axis_cvops.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/axis_engineer 
{
	sort 16
	{
		map models/players/hud/axis_engineer.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/axis_field 
{
	sort 16
	{
		map models/players/hud/axis_field.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/axis_soldier 
{
	sort 16
	{
		map models/players/hud/axis_soldier.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

//****************************************
//	
//	Allies body
//
//*****************************************
models/players/temperate/allied/soldier/body
{
	sort 16
	{
		map models/players/temperate/allied/soldier/body.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/medic/body
{
	sort 16
	{
		map models/players/temperate/allied/medic/body.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/engineer/body
{
	sort 16
	{
		map models/players/temperate/allied/engineer/body.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/fieldops/body
{
	sort 16
	{
		map models/players/temperate/allied/fieldops/body.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/cvops/body
{
	sort 16
	{
		map models/players/temperate/allied/cvops/body.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

///****************************************
//	
//	Allies pants
//
//*****************************************
models/players/temperate/allied/leg01 
{
	sort 16
	{
		map models/players/temperate/allied/leg01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

///***************************************** 
//	
//	Allies backpacks
//
//*****************************************
models/players/temperate/allied/medic/acc/backpack 
{
	sort 16	
	{
		map models/players/temperate/allied/medic/acc/backpack.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/medic/acc/backpack2 
{
	sort 16	
	{
		map models/players/temperate/allied/medic/acc/backpack2.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/engineer/acc/backpack 
{
	sort 16	
	{
		map models/players/temperate/allied/engineer/acc/backpack.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/engineer/acc/shovel 
{
	sort 16	
	{
		map models/players/temperate/allied/engineer/acc/shovel.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/fieldops/acc/backpack 
{
	sort 16	
	{
		map models/players/temperate/allied/fieldops/acc/backpack.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/soldier/acc/backpack 
{
	sort 16	
	{
		map models/players/temperate/allied/soldier/acc/backpack.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

//******************************************
//	
//	Allies helmets
//
//******************************************
models/players/temperate/allied/medic/helmet
{
	sort 16
	{
		map models/players/temperate/allied/medic/helmet.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/cvops/cap 
{
	sort 16
	{
		map models/players/temperate/allied/cvops/cap.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/engineer/helmet 
{
	sort 16
	{
		map models/players/temperate/allied/engineer/helmet.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/fieldops/helmet 
{
	sort 16
	{
		map models/players/temperate/allied/fieldops/helmet.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/soldier/helmet 
{
	sort 16
	{
		map models/players/temperate/allied/soldier/helmet.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/strap
{
	sort 16
	{
		map models/players/temperate/allied/inside.tga 
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/temperate/allied/inside
{
	sort 16
	{
		map models/players/temperate/allied/inside.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

//*******************************************
//	
//	Allies heads
//
//*******************************************
models/players/hud/allied_medic 
{
	sort 16
	{
		map models/players/hud/allied_medic.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/allied_cvops 
{
	sort 16	
	{
		map models/players/hud/allied_cvops.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/allied_engineer 
{
	sort 16
	{
		map models/players/hud/allied_engineer.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/allied_field 
{
	sort 16
	{
		map models/players/hud/allied_field.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/allied_soldier 
{
	sort 16
	{
		map models/players/hud/allied_soldier.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/teeth01 
{
	/* lets hide teeth, so it won't look ugly with transparency */
    {
    	map models/players/hud/teeth01.tga
    	blendFunc GL_ZERO GL_ONE
    }
}

models/players/hud/eye01 
{	
	{
		map models/players/hud/eye01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/eye02 
{	
	{
		map models/players/hud/eye02.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

models/players/hud/eye03 
{	
	{
		map models/players/hud/eye03.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen normalzfade 1 -200 200
		rgbGen lightingdiffuse
		depthWrite
	}
}

// alternative simple players representation through single shader
etjump/simple_player
{
    cull none
    {
        map models/players/ghost.tga
		tcmod scale 4 6
		tcmod scroll 0 -0.1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen entity
        alphaGen entity
    }
}