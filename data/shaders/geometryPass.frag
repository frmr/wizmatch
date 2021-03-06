#version 150

varying vec3 		normals;
uniform sampler2D	surfaceTexture;

void main( void )
{
	gl_FragData[0] = vec4( ( normals.xyz + 1.0 ) * 0.5, 0.0 ); //pack normals into the range 0 to 1
	if ( texture2D( surfaceTexture, gl_TexCoord[0].st ).a < 0.1 )
	{
		discard;
	}
	else
	{
		gl_FragData[1] = vec4( texture2D( surfaceTexture, gl_TexCoord[0].st ).rgb, 0 );
	}
}
