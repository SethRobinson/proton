package com.rtsoft
{
	
	/*
	
	   GLES1Adaptor
	   Used with GLFlashAdaptor.cpp, which implements a subset of GLES 1.1 calls.  Lots of gotchas, so check out the info on protonsdk.com
	
	   (c) Seth A. Robinson 2012:  All Rights reserved
	
	 */
	
	import flash.display.*;
	import flash.display3D.*;
	import flash.display3D.textures.Texture;
	import flash.text.TextField;
	import flash.net.LocalConnection;
	import flash.net.URLRequest;
	import C_Run.ram;
	import C_Run.initLib;
	import flash.geom.*;
	import flash.utils.*;
	import flash.events.*;
	import flash.geom.Matrix3D;
	import flash.geom.Rectangle;
	import com.adobe.utils.*;
	import flash.system.*;
	import com.rtsoft.*;
	
	public class GLES1Adaptor
	{
		
		//Pos textured
		private var programPosTextured:Program3D;
		private var vertexShaderPosTextured:ByteArray;
		private var fragmentShaderTextured:ByteArray;
		
		//Pos textured with vertex color
		private var programPosTexturedVertexColor:Program3D;
		private var vertexShaderPosTexturedVertexColor:ByteArray;
		private var fragmentShaderTexturedVertexColor:ByteArray;
		
		//Pos untextured with single color using glColor
		private var programPos:Program3D;
		private var vertexShaderPos:ByteArray;
		private var fragmentShaderBasic:ByteArray;
		
		private var programPosTexturedNormals:Program3D;
		private var vertexShaderPosTexturedNormals:ByteArray;
		private var fragmentShaderTexturedNormals:ByteArray;
		
		private var programPosColors:Program3D;
		private var vertexShaderPosColors:ByteArray;
		private var fragmentShaderColors:ByteArray;
		
		private var programPosColorsNormals:Program3D;
		private var vertexShaderPosColorsNormals:ByteArray;
		private var fragmentShaderColorsNormals:ByteArray;
		
		private var m_polyColor:Vector.<Number>;
		
		private var m_surfaceDict:Dictionary = new Dictionary();
		private var m_textureGenCounter:int = 0;
		
		private var m_VBODict:Dictionary = new Dictionary();
		private var m_VBOGenCounter:int = 0;
		
		public static var current:GLES1Adaptor;
		public var context:Context3D;
		public var m_debug:Boolean = false;
		
		public function GLES1Adaptor(con:Context3D)
		{
			context = con;
			GLES1Adaptor.current = this
			
			//setup the defaults
			
			SetPolyColor(1, 1, 1, 1);
			context.setCulling(Context3DTriangleFace.BACK);
			createAndCompileProgramPos(); //position and vc0 for the color
			createAndCompileProgramPosTextured(); //position and a texture for the color
			createAndCompileProgramPosTexturedVertexColor();
			createAndCompileProgramPosTexturedNormals(); //position and a texture for the color + normals with 1 static light
			createAndCompileProgramPosColors(); //position and vertex colors
			createAndCompileProgramPosColorsNormals(); //position, vertex colors and normals with 1 static light
			var lightDirection:Vector3D = new Vector3D(1.5, 1, 1, 0);
			
			//some defaults
			lightDirection.normalize();
			context.setProgramConstantsFromVector(Context3DProgramType.FRAGMENT, 2, Vector.<Number>([0, 0, 0, 0])); //fc2, for clamping negative values to zero
			context.setProgramConstantsFromVector(Context3DProgramType.FRAGMENT, 3, Vector.<Number>([1, 1, 1, 1])); //default diffuse
			context.setProgramConstantsFromVector(Context3DProgramType.FRAGMENT, 15, Vector.<Number>([lightDirection.x, lightDirection.y, lightDirection.z, 0]));
			context.setProgramConstantsFromVector(Context3DProgramType.FRAGMENT, 16, Vector.<Number>([0.4, 0.4, 0.4, 1])); //fc1, ambient lighting (1/4 of full intensity)
		
		}
		
		private function createAndCompileProgramPos():void
		{
			programPos = context.createProgram();
			
			// Create an AGALMiniAssembler.
			var assembler:AGALMiniAssembler = new AGALMiniAssembler();
			
			// VERTEX SHADER
			var code:String = "";
			code += "m44 op, va0, vc0\n"; //4x4 matrix transform from 0 to output clipspace
			
			// Compile our AGAL Code into ByteCode using the MiniAssembler 
			vertexShaderPos = assembler.assemble(Context3DProgramType.VERTEX, code);
			
			code = "mov oc, fc0\n"; // Move the Variable register 0 (v0) where we copied our Vertex Color, to the output color
			
			// Compile our AGAL Code into Bytecode using the MiniAssembler
			fragmentShaderBasic = assembler.assemble(Context3DProgramType.FRAGMENT, code);
			
			// UPLOAD TO GPU PROGRAM
			programPos.upload(vertexShaderPos, fragmentShaderBasic); // Upload the combined program to the video Ram
		}
		
		public function GenTexture():int
		{
			m_textureGenCounter++;
			//trace("Creating texid " + m_textureGenCounter);
			m_surfaceDict[m_textureGenCounter] = Texture;
			return m_textureGenCounter;
		}
		
		public function GenVBO():int
		{
			m_VBOGenCounter++;
			//trace("Creating vertex buffer " +m_VBOGenCounter);
			return m_VBOGenCounter;
		}
		
		public function DeleteTexture(texID:int)
		{
			if (m_debug)
			{
				trace("Flash uncaching texture " + texID);
			}
			delete m_surfaceDict[texID];
		}
		
		public function DeleteVBO(vboId:int)
		{
			delete m_VBODict[vboId];
		}
		
		//defines from GLES
		private static const GL_RGB:int = 6407;
		private static const GL_RGBA:int = 6408;
		private static const GL_UNSIGNED_BYTE:int = 5121;
		
		public function SetTexture(slotNum:int, bActive:Boolean, texID:int)
		{
			if (!bActive)
			{
				//trace("Disabling tex slot " + slotNum);
				context.setTextureAt(slotNum, null);
			}
			else
			{
				//trace("Binding tex "+texID+" to slot " + slotNum);
				context.setTextureAt(slotNum, m_surfaceDict[texID]);
			}
		}
		
		public function LoadTexture(texID:int, mipLevel:int, w:int, h:int, pixelPtr:int):void
		{
			if (m_debug)
			{
				trace("Flash wants to create a " + w + " by " + h + " tex at mip level " + mipLevel + " for texid " + texID);
			}
			
			//********** Create image and set it ********
			m_surfaceDict[texID] = context.createTexture(w, h, Context3DTextureFormat.BGRA, false);
			m_surfaceDict[texID].uploadFromByteArray(ram, pixelPtr);
			//****************************
		}
		
		private function createAndCompileProgramPosTextured():void
		{
			programPosTextured = context.createProgram();
			
			// Create an AGALMiniAssembler.
			var assembler:AGALMiniAssembler = new AGALMiniAssembler();
			
			// VERTEX SHADER
			var code:String = "";
			code += "m44 op, va0, vc0\n";
			// tell fragment shader about XYZ
			code += "mov v0, va0\n";
			// tell fragment shader about UV
			code += "mov v1, va1\n"; //copy texcoord from 1 to fragment program
			
			// Compile our AGAL Code into ByteCode using the MiniAssembler 
			vertexShaderPosTextured = assembler.assemble(Context3DProgramType.VERTEX, code);
			
			// and uv coordinates from varying register 1.  clamp/repeat
			code = "tex ft0, v1, fs0 <2d,repeat,linear>\n"; //we'll need clamp and miplinear later...
			// multiply by the value stored in fc0
			code += "mul ft1, fc0, ft0\n";
			// move this value to the output color
			code += "mov oc, ft1\n"
			
			// Compile our AGAL Code into Bytecode using the MiniAssembler
			fragmentShaderTextured = assembler.assemble(Context3DProgramType.FRAGMENT, code);
			
			// UPLOAD TO GPU PROGRAM
			programPosTextured.upload(vertexShaderPosTextured, fragmentShaderTextured); // Upload the combined program to the video Ram
		}
		
		//Um, written but untested
		private function createAndCompileProgramPosTexturedNormals():void
		{
			programPosTexturedNormals = context.createProgram();
			
			// Create an AGALMiniAssembler.
			var assembler:AGALMiniAssembler = new AGALMiniAssembler();
			
			// VERTEX SHADER
			var code:String = "";
			code += "m44 op, va0, vc0\n";
			// tell fragment shader about XYZ
			code += "mov v0, va0\n";
			// tell fragment shader about UV
			code += "mov v2, va1\n"; //copy texcoord from 1 to fragment program
			//tell fragment shader about normals
			code += "mov v3, va3\n";
			
			//HANDLE NORMAL
			code += "m44 vt0, va3, vc4\n"; // multiply the normal with the special normat matrix to temp
			code += "nrm vt1.xyz, vt0.xyz\n"; //normalize the resultant normal, as the matrix may have screwed up its scale
			code += "mov vt1.w, vc4.w\n"; //avoid error about w not being set
			code += "mov v3, vt1\n"; // Interpolate the normal (va1) into variable register v3
			
			// Compile our AGAL Code into ByteCode using the MiniAssembler 
			vertexShaderPosTexturedNormals = assembler.assemble(Context3DProgramType.VERTEX, code);
			
			// grab the texture color from texture 0 
			// and uv coordinates from varying register 1
			code = "tex ft0, v2, fs0 <2d,repeat,linear>\n";
			
			//fc16 = ambient
			//fc15 = light direction
			//fc2 = 0,0,0,0
			//fc3 = light color/diffuse
			code += "dp3 ft1.x, fc15, v3 \n"; // dot the transformed normal (v1) with light direction fc2 -&gt; This is the Lamberian Factor
			code += "sat ft1.x, ft1.x\n"; //clamp between 0 and 1
			
			//test, remove light (light is 0 power on every pixel)
			//code += "mov ft1.x, fc2.x \n"; 				
			
			//test, force all light (light is max power on every pixel)
			//code += "mov ft1.x, fc3.x \n"; 				
			
			code += "mul ft2.rgb, ft0.rgb, ft1.x \n"; //multiply fragment color (ft0) by light amount (ft1.x).
			
			//add light coloring
			
			code += "mul ft3.rgb, ft0.rgb, fc16.rgb \n"; //multiply fragment color (ft0) by light amount (ft1).
			
			code += "mov ft2.w, ft0.w\n"; //directly copy alpha from our source vertex color
			
			//now here is how we determine how we mix ambient and the final light processed color
			code += "max oc.rgba, ft2.rgba, ft3.rgba\n";
			
			// Compile our AGAL Code into Bytecode using the MiniAssembler
			fragmentShaderTexturedNormals = assembler.assemble(Context3DProgramType.FRAGMENT, code);
			
			// UPLOAD TO GPU PROGRAM
			programPosTexturedNormals.upload(vertexShaderPosTexturedNormals, fragmentShaderTexturedNormals); // Upload the combined program to the video Ram
		}
		
		private function createAndCompileProgramPosColors():void
		{
			programPosColors = context.createProgram();
			
			// Create an AGALMiniAssembler.
			var assembler:AGALMiniAssembler = new AGALMiniAssembler();
			
			// VERTEX SHADER
			var code:String = "";
			code += "m44 op, va0, vc0\n";
			// tell fragment shader about XYZ
			code += "mov v0, va0\n";
			// tell fragment shader about vertex colors
			code += "mov v2, va2\n";
			
			// Compile our AGAL Code into ByteCode using the MiniAssembler 
			vertexShaderPosColors = assembler.assemble(Context3DProgramType.VERTEX, code);
			
			// grab the texture color from texture 0 
			// and uv coordinates from varying register 1
			code = "mov ft0, v2\n";
			// multiply by the value stored in fc0
			code += "mul ft1, fc0, ft0\n";
			// move this value to the output color
			code += "mov oc, ft1\n"
			
			// Compile our AGAL Code into Bytecode using the MiniAssembler
			fragmentShaderColors = assembler.assemble(Context3DProgramType.FRAGMENT, code);
			
			// UPLOAD TO GPU PROGRAM
			programPosColors.upload(vertexShaderPosColors, fragmentShaderColors); // Upload the combined program to the video Ram
		}
		
		private function createAndCompileProgramPosColorsNormals():void
		{
			programPosColorsNormals = context.createProgram();
			
			// Create an AGALMiniAssembler.
			var assembler:AGALMiniAssembler = new AGALMiniAssembler();
			
			// VERTEX SHADER
			var code:String = "";
			code += "m44 op, va0, vc0\n";
			// tell fragment shader about XYZ
			code += "mov v0, va0\n";
			// tell fragment shader about UV
			//code += "mov v1, va1\n"; //copy texcoord from 1 to fragment program
			// tell fragment shader about vertex colors
			code += "mov v2, va2\n";
			//tell fragment shader about normals
			code += "mov v3, va3\n";
			
			//HANDLE NORMAL
			code += "m44 vt0, va3, vc4\n"; // multiply the normal with the special normat matrix to temp
			code += "nrm vt1.xyz, vt0.xyz\n"; //normalize the resultant normal, as the matrix may have screwed up its scale
			code += "mov vt1.w, vc4.w\n"; //avoid error about w not being set
			code += "mov v3, vt1\n"; // Interpolate the normal (va1) into variable register v3
			
			// Compile our AGAL Code into ByteCode using the MiniAssembler 
			vertexShaderPosColorsNormals = assembler.assemble(Context3DProgramType.VERTEX, code);
			
			// grab the vertex color
			code = "mov ft0, v2\n";
			
			//fc16 = ambient
			//fc15 = light direction
			//fc2 = 0,0,0,0
			//fc3 = light color/diffuse
			code += "dp3 ft1.x, fc15, v3 \n"; // dot the transformed normal (v1) with light direction fc2 -&gt; This is the Lamberian Factor
			code += "sat ft1.x, ft1.x\n"; //clamp between 0 and 1
			
			//test, remove light (light is 0 power on every pixel)
			//code += "mov ft1.x, fc2.x \n"; 				
			
			//test, force all light (light is max power on every pixel)
			//code += "mov ft1.x, fc3.x \n"; 				
			
			code += "mul ft2.rgb, ft0.rgb, ft1.x \n"; //multiply fragment color (ft0) by light amount (ft1.x).
			
			//add light coloring
			
			code += "mul ft3.rgb, ft0.rgb, fc16.rgb \n"; //multiply fragment color (ft0) by light amount (ft1).
			
			code += "mov ft2.w, ft0.w\n"; //directly copy alpha from our source vertex color
			
			//now here is how we determine how we mix ambient and the final light processed color
			code += "max oc.rgba, ft2.rgba, ft3.rgba\n";
			
			// Compile our AGAL Code into Bytecode using the MiniAssembler
			fragmentShaderColorsNormals = assembler.assemble(Context3DProgramType.FRAGMENT, code);
			
			// UPLOAD TO GPU PROGRAM
			programPosColorsNormals.upload(vertexShaderPosColorsNormals, fragmentShaderColorsNormals); // Upload the combined program to the video Ram
		}
		
		private function createAndCompileProgramPosTexturedVertexColor():void
		{
			programPosTexturedVertexColor = context.createProgram();
			
			// Create an AGALMiniAssembler.
			var assembler:AGALMiniAssembler = new AGALMiniAssembler();
			
			// VERTEX SHADER
			var code:String = "";
			code += "m44 op, va0, vc0\n";
			// tell fragment shader about XYZ
			code += "mov v0, va0\n";
			// tell fragment shader about UV
			code += "mov v1, va1\n"; //copy texcoord from 1 to fragment program
			code += "mov v2, va2\n"; //copy vertex color
			//code += "mov v2, va2\n"; //copy vertex color
			
			// Compile our AGAL Code into ByteCode using the MiniAssembler 
			vertexShaderPosTexturedVertexColor = assembler.assemble(Context3DProgramType.VERTEX, code);
			
			// and uv coordinates from varying register 1
			code = "tex ft0, v1, fs0 <2d,repeat,linear>\n"; //we'll need clamp and miplinear later...
			// multiply by the vertec color we stored before
			code += "mul ft1, v2, ft0\n";
			// move this value to the output color
			code += "mov oc, ft1\n"
			
			// Compile our AGAL Code into Bytecode using the MiniAssembler
			fragmentShaderTexturedVertexColor = assembler.assemble(Context3DProgramType.FRAGMENT, code);
			
			// UPLOAD TO GPU PROGRAM
			programPosTexturedVertexColor.upload(vertexShaderPosTexturedVertexColor, fragmentShaderTexturedVertexColor); // Upload the combined program to the video Ram
		}
		
		public function GetMatrixFromPtr(matrixPtr:int):Matrix3D
		{
			ram.position = matrixPtr;
			return new Matrix3D(Vector.<Number>([ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat(), ram.readFloat()]));
		}
		
		public function SetPolyColor(r:Number, g:Number, b:Number, a:Number):void
		{
			if (m_debug)
			{
			//	trace("setting poly color r " + r + " and alpha: " + a);
			}
			m_polyColor = Vector.<Number>([r, g, b, a]);
		}
		
		public function SetLightAmbient(lightID:int, r:Number, g:Number, b:Number):void
		{
			/*
			   r = 0;
			   g = 0;
			   b = 0;
			 */
			if (m_debug)
			{
				//trace("Setting light ambient to " + r + ", " + g + ", " + b);
			}
			
			context.setProgramConstantsFromVector(Context3DProgramType.FRAGMENT, 16, Vector.<Number>([r, g, b, 0])); //fc1, ambient lighting (1/4 of full intensity)
		}
		
		public function SetLightDiffuse(lightID:int, r:Number, g:Number, b:Number):void
		{
			if (m_debug)
			{
				//trace("Setting light diffuse to " + r + ", " + g + ", " + b);
			}
			
			context.setProgramConstantsFromVector(Context3DProgramType.FRAGMENT, 3, Vector.<Number>([r, g, b, 1])); // Light Color
		}
		
		public function SetLightDir(lightID:int, x:Number, y:Number, z:Number):void
		{
			var lightDirection:Vector3D = new Vector3D(x, y, z, 0);
			
			//lightDirection = new Vector3D(0.0, 0, -10.0, 0);
			
			lightDirection.normalize();

			if (m_debug)
			{
				//trace("****** Lighting final is " + lightDirection);
			}
			context.setProgramConstantsFromVector(Context3DProgramType.FRAGMENT, 15, Vector.<Number>([lightDirection.x, lightDirection.y, lightDirection.z, 0]));
		}
		
		public function SetVBO(vboId:int, vertPtr:int, vertCount:Number, dataSizePerVertexInts:int):void
		{
			//trace("Setting VBO " + vboId + " with " + vertCount + " verts and a datasize of " + dataSizePerVertexInts);
			delete m_VBODict[vboId];
			m_VBODict[vboId] = VertexBuffer3D;
			m_VBODict[vboId] = context.createVertexBuffer(vertCount, dataSizePerVertexInts);
			m_VBODict[vboId].uploadFromByteArray(ram, vertPtr, 0, vertCount);
		}
		
		public function SetIndexVBO(vboId:int, vertPtr:int, vertCount:Number):void
		{
			//OPTIMIZE - is this delete slow?  If figured it's ignored if not needed.. right?!
			delete m_VBODict[vboId];
			m_VBODict[vboId] = IndexBuffer3D;
			m_VBODict[vboId] = context.createIndexBuffer(vertCount);
			m_VBODict[vboId].uploadFromByteArray(ram, vertPtr, 0, vertCount);
		}
		
		public function AttachVBOVertsToShader(vboId:int, buffOffsetInt:int)
		{
			context.setVertexBufferAt(0, m_VBODict[vboId], buffOffsetInt, Context3DVertexBufferFormat.FLOAT_3); // register "0" now contains x,y,z
		}
		
		public function AttachVBOTexUVToShader(vboId:int, buffOffsetInt:int)
		{
			context.setVertexBufferAt(1, m_VBODict[vboId], buffOffsetInt, Context3DVertexBufferFormat.FLOAT_2); // register "1" now contains uvs
		}
		
		public function AttachVBOColorsToShader(vboId:int, buffOffsetInt:int)
		{
			//colors in the form of 255 255 255 255 in a single uint32
			context.setVertexBufferAt(2, m_VBODict[vboId], buffOffsetInt, Context3DVertexBufferFormat.BYTES_4);
		}
		
		public function AttachVBONormalsToShader(vboId:int, buffOffsetInt:int)
		{
			//colors in the form of 255 255 255 255 in a single uint32
			context.setVertexBufferAt(3, m_VBODict[vboId], buffOffsetInt, Context3DVertexBufferFormat.FLOAT_3);
		}
		
		//Draws triangle list of solid polys with a single color
		public function DrawArraysMulti(matrixPtr:int, modelMatrixPtr:int, indexVBO:int, bUsingUVs:Boolean, bUsingNormals:Boolean, bUsingVertexColors:Boolean):void
		{
			//trace("DrawArraysPosMulti " + vertCount + " verts and " + indexCount + " indexes.");
			context.setProgramConstantsFromVector(Context3DProgramType.FRAGMENT, 0, m_polyColor);
			context.setProgramConstantsFromMatrix(Context3DProgramType.VERTEX, 0, GetMatrixFromPtr(matrixPtr), true); //set the matrix we were given so verts will transform in HW with it
			
			//choose best shader for the job
			
			if (bUsingUVs)
			{
				if (bUsingVertexColors)
				{
					//with vertex colors...
					if (bUsingNormals)
					{
						trace("Don't support textured vertex colored with normals yet.. add shader");
						//context.setProgram(programPosTexturedVertexColorNormals); //untested.. but should work.. (famous last words)
					}
					else
					{
						context.setProgram(programPosTexturedVertexColor);
					}
				}
				else
				{
					if (bUsingNormals)
					{
						context.setProgram(programPosTexturedNormals);
					}
					else
					{
						context.setProgram(programPosTextured);
					}
				}
			}
			else
			{
				
				context.setVertexBufferAt(1, null); //clear uv data
				
				//not using UVs.. but we still have other options to consider
				
				if (bUsingVertexColors)
				{
					
					if (bUsingNormals)
					{
						//context.setProgramConstantsFromVector(Context3DProgramType.FRAGMENT, 1, Vector.<Number>([0.1,0.1,0.1,0])); //fc1, ambient lighting (1/4 of full intensity)
						
						context.setProgram(programPosColorsNormals);
						//setup matrix so we can rotate the normals in the shader
						var normalMat:Matrix3D = new Matrix3D();
						normalMat.copyFrom(GetMatrixFromPtr(modelMatrixPtr));
						normalMat.position = new Vector3D(0, 0, 0); //remove translation
						context.setProgramConstantsFromMatrix(Context3DProgramType.VERTEX, 4, normalMat, true); //set normal mat, keeping mind it will take 4 registers
					}
					else
					{
						context.setProgram(programPosColors);
					}
				}
				else
				{
					if (bUsingNormals)
					{
						if (m_debug)
						{
								trace("Don't handle pos + normals yet");
						}
					}
					{
						context.setProgram(programPos);
					}
				}
			}
			
			//context.setCulling(Context3DTriangleFace.NONE);
			context.drawTriangles(m_VBODict[indexVBO]); // Draw the triangle according to the indexBuffer instructions into the backbuffer
			
			if (bUsingVertexColors)
			{
				context.setVertexBufferAt(2, null); //clear old data 
			}
			if (bUsingNormals)
			{
				context.setVertexBufferAt(3, null); //clear old data or it won't work
			}
		}

		public function SetDebugMode(debug:Boolean)
		{
			m_debug = debug;
			// By enabling the Error reporting, you can get some valuable information about errors in your shaders
			// But it also dramatically slows down your program.
			trace("Enabling debug mode");
			context.enableErrorChecking = debug;
		}
	}
}
