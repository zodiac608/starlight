/******************************************************************************
file: rn_t.cpp
desc: 
 *****************************************************************************/
#include "rn_t.h"			//
#include "common.h"			//
#include "fs_t.h"			//filesystem
#include "log_t.h"			//for logging
#include <GL/glew.h>		//extension enumerator
#include <GL/gl.h>			//gl types
#include <math.h>			//fmodf, fcosf, fsinf
#include <vector>			//stl vector
#include <algorithm>		//stl for each
#include <string>			//stl string
#include <string.h>			//memset

using std::vector;
using std::string;
using std::for_each;

extern log_t* g_log;

#define RIGHT_EXTENT 0.8f
#define LEFT_EXTENT -RIGHT_EXTENT
#define TOP_EXTENT 0.20f
#define MIDDLE_EXTENT 0.0f
#define BOTTOM_EXTENT -TOP_EXTENT
#define FRONT_EXTENT -1.25f
#define REAR_EXTENT -1.75f

#define GREEN_COLOR 0.75f, 0.75f, 1.0f, 1.0f
#define BLUE_COLOR 	0.0f, 0.5f, 0.0f, 1.0f
#define RED_COLOR 1.0f, 0.0f, 0.0f, 1.0f
#define GREY_COLOR 0.8f, 0.8f, 0.8f, 1.0f
#define BROWN_COLOR 0.5f, 0.5f, 0.0f, 1.0f

int g_num_vtx = 36;
const float vtx_data[] =
{
	//Object 1 positions
	LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,

	LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,
	LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,
	LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	//	0, 2, 1,
	//	3, 2, 0,

	//Object 2 positions
	TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
	MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
	TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,

	BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
	MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
	BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

	TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
	BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,

	TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
	BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

	BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,
	TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
	TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,
	BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

	//Object 1 colors
	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,

	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,

	RED_COLOR,
	RED_COLOR,
	RED_COLOR,

	GREY_COLOR,
	GREY_COLOR,
	GREY_COLOR,

	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,

	//Object 2 colors
	RED_COLOR,
	RED_COLOR,
	RED_COLOR,
	RED_COLOR,

	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,

	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,

	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,

	GREY_COLOR,
	GREY_COLOR,
	GREY_COLOR,
	GREY_COLOR,
};

const GLushort index_data[] =
{
	0, 2, 1,
	3, 2, 0,

	4, 5, 6,
	6, 7, 4,

	8, 9, 10,
	11, 13, 12,

	14, 16, 15,
	17, 16, 14,
};

/******************************************************************************
func: constructor, zero out the perspective array and set the initial scales
desc: 
 *****************************************************************************/
rn_t::rn_t( void )
{
	memset( r_vao_ar, 0, sizeof(GLint) * MAX_VAOS );
	memset( ps_matrix, 0, sizeof(GLfloat) * 16 );
	r_used_vaos = 0;

	f_scale = 1.0f;
	z_near = 1.0f;
	z_far = 3.0f;
}


/******************************************************************************
func:
desc: start the renderer 
 *****************************************************************************/
int rn_t::start( void )
{
	int retv = ERR_OK;
	GLenum gl_err = glewInit();

	/*start glew*/
	if( gl_err != GLEW_OK )
	{
		g_log->w( MSG_FATAL, "error starting glew (%s)\n",
				glewGetErrorString( gl_err ) );
		retv = ERR_GLEW;
		goto out;
	}

	g_log->w( MSG_INFO, "glew version %s started successfully\n",
			glewGetString( GLEW_VERSION ) );

	/*setup the shaders*/
	if( ( retv = load_all_shaders() ) != ERR_OK )
		goto out;

	if( ( retv = create_sh_program() ) != ERR_OK )
		goto out;

	/*initialze shaders*/
	initialize_shaders();
	delete_all_shaders();

	/*initialize the vertex data*/
	init_vtx_b();
	init_vaos();

	/*set desired gl state*/
	set_gl_state();

out:
	return retv;
}


/******************************************************************************
func:
desc: draw the scene
 *****************************************************************************/
int rn_t::draw( void )
{
	int i = 0;
	unsigned color_data = sizeof( vtx_data ) / 2;
	static GLfloat offx = 0.0;
	static GLfloat offy = 0.0;
	GLfloat offz = 0.0;

	glClearColor( BG_COLOR );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glUseProgram( r_sh_program );

	for( i = 0; i < r_used_vaos; i ++ )
	{
		glBindVertexArray( r_vao_ar[i] );
		glUniform3f(
				glGetUniformLocation( r_sh_program, "offset" ),
				offx,
				offy,
				offz );
		glDrawElements(
				GL_TRIANGLES, ARRAY_COUNT( index_data ), GL_UNSIGNED_SHORT, 0 );
		offz = 0.50f;
	}

	glBindVertexArray( 0 );
	glUseProgram( 0 );

	offx += 0.01f;
	offy += 0.005f;

	if( offx > 1.0f )
		offx = -1.0f;
	if( offy > 1.0f )
		offy = -1.0f;

	return ERR_OK;
}


/******************************************************************************
func:
desc: 
 *****************************************************************************/
int rn_t::shutdown( void )
{
	/*shut down*/
	glDeleteProgram( r_sh_program );
	return ERR_OK;
}


/******************************************************************************
func: take care of a window resize call
desc:
 *****************************************************************************/
int rn_t::resize_gl( int w, int h )
{
	int retv = ERR_OK;
	GLuint perspective_mat_unif = 0;

	if( w <= 0 || h <= 0 )
	{
		retv = ERR_WIN_SIZE;
		goto out;
	}

	/*set the new scale*/
	ps_matrix[0] = f_scale / ( w / (float)h );
	ps_matrix[5] = f_scale;

	/*send it to the shader*/
	perspective_mat_unif = glGetUniformLocation( r_sh_program,
			"perspective_matrix" );

	/*send the matrix to the shader*/
	glUseProgram( r_sh_program );
	glUniformMatrix4fv( perspective_mat_unif, 1, GL_FALSE, ps_matrix );
	glUseProgram( 0 );

	/*reset the viewport*/
	glViewport( 0, 0, (GLsizei)w, (GLsizei)h );

out:
	return retv;
}


/******************************************************************************
func:
desc: 
 *****************************************************************************/
int rn_t::load_all_shaders( void )
{
	int retv = ERR_OK;
	GLuint sh_num = 0;
	fs_t f;
	vector<string> files;
	vector<string>::iterator i = files.begin();
	string current_shader = "";

	/*get all vertex the files*/
	f.get_all_files_in_dir( VTX_SHADER_DIR, &files );

	/*compile all the vertex shaders*/
	for( i = files.begin(); i != files.end(); i ++ )
	{
		current_shader = "";
		f.read_file_to_string( *i, &current_shader );
		if( ( retv = compile_shader(
						&sh_num,
						GL_VERTEX_SHADER,
						(char*)current_shader.c_str(),
						*i ) ) != ERR_OK )
			goto out;

		r_shader_list.push_back( sh_num );
	}

	g_log->w( MSG_INFO, "found %u vertex shaders\n", files.size() );

	files.clear();

	/*do the same for the fragment shaders*/
	f.get_all_files_in_dir( FRAG_SHADER_DIR, &files );

	for( i = files.begin(); i != files.end(); i ++ )
	{
		current_shader = "";
		f.read_file_to_string( *i, &current_shader );
		if( ( retv = compile_shader(
						&sh_num,
						GL_FRAGMENT_SHADER,
						(char*)current_shader.c_str(),
						*i ) ) != ERR_OK )
			goto out;

		r_shader_list.push_back( sh_num );
	}

	g_log->w( MSG_INFO, "found %u fragment shaders\n", files.size() );

out:
	return retv;
}


/******************************************************************************
func: compile_shader
desc: compile an individual shader for opengl
 *****************************************************************************/
int rn_t::compile_shader( GLuint* sh_num,
		GLenum sh_type,
		char* sh_file,
		string sh_file_name )
{
	int retv = ERR_OK;
	GLuint shader = glCreateShader( sh_type );
	GLint status = 0;
	GLint log_length = 0;
	GLchar* shader_err_log = NULL;
	assert( sh_num && sh_file && shader );

	/*compile the shader*/
	glShaderSource( shader, 1, (const char**)&sh_file, NULL );
	glCompileShader( shader );

	/*check for errors*/
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if( status == GL_FALSE )
	{
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &log_length );
		shader_err_log = new GLchar[log_length + 1];
		glGetShaderInfoLog( shader, log_length, NULL, shader_err_log );

		g_log->w ( MSG_FATAL,
				"error compiling shader %s (",
				sh_file_name.c_str() );

		switch( sh_type )
		{
			case GL_VERTEX_SHADER: g_log->w( "vertex" ); break;
			case GL_GEOMETRY_SHADER: g_log->w( "geometry" ); break;
			case GL_FRAGMENT_SHADER: g_log->w( "fragment" ); break;
			default: g_log->w( "unknown" ); break;
		}

		g_log->w( ")\n\n%s", shader_err_log );
		delete[] shader_err_log;
		shader_err_log = NULL;
		retv = ERR_SHADER_COMPILE;
		goto out;
	}

out:
	*sh_num = shader;
	return retv;
}


/******************************************************************************
func:
desc: 
 *****************************************************************************/
int rn_t::create_sh_program( void )
{
	int retv = ERR_OK;
	GLint glerr = 0;
	GLint glerr_len = 0;
	GLchar* glerr_log = NULL;
	vector<GLuint>::iterator i = r_shader_list.begin();

	r_sh_program = glCreateProgram();

	/*attach all the shaders to the program*/
	while( i != r_shader_list.end() )
	{
		glAttachShader( r_sh_program, *i );
		i++;
	}

	/*link it*/
	glLinkProgram( r_sh_program );

	/*check for errors*/
	glGetProgramiv( r_sh_program, GL_LINK_STATUS, &glerr );
	if( glerr == GL_FALSE )
	{
		glGetProgramiv( r_sh_program, GL_INFO_LOG_LENGTH, &glerr_len );
		glerr_log = new GLchar[glerr_len + 1];
		glGetProgramInfoLog( r_sh_program, glerr_len, NULL, glerr_log );
		g_log->w( "shader linker failure: %s\n", glerr_log );
		delete[] glerr_log;
		glerr_log = NULL;
		retv = ERR_SHADER_LINK;
	}

	/*detach the shaders*/
	for( i = r_shader_list.begin(); i != r_shader_list.end(); i++ )
		glDetachShader( r_sh_program, *i );

	return retv;
}


/******************************************************************************
func:
desc: 
 *****************************************************************************/
int rn_t::delete_all_shaders( void )
{
	for_each( r_shader_list.begin(), r_shader_list.end(), glDeleteShader );
	return ERR_OK;
}


/******************************************************************************
func:
desc: 
 *****************************************************************************/
int rn_t::init_vtx_b( void )
{
	/*create the buffer object for the verticies*/
	glGenBuffers( 1, &r_pos_bobj );
	glBindBuffer( GL_ARRAY_BUFFER, r_pos_bobj );
	glBufferData(
			GL_ARRAY_BUFFER,
			sizeof(vtx_data),
			vtx_data,
			GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	/*create the buffer object for the indicies*/
	glGenBuffers( 1, &r_index_bobj );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, r_index_bobj );
	glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			sizeof(index_data),
			index_data,
			GL_STATIC_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	return ERR_OK;
}


/******************************************************************************
func:
desc: setup all the vaos we want to use
 *****************************************************************************/
int rn_t::init_vaos( void )
{
	size_t color_data_offset = 0;
	size_t pos_data_offset = 0;

	/*generate the first vao*/
	color_data_offset = sizeof(float) * 3 * g_num_vtx;

	glGenVertexArrays( 1, &r_vao_ar[0] );
	glBindVertexArray( r_vao_ar[0] );
	glBindBuffer( GL_ARRAY_BUFFER, r_pos_bobj );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			0 );

	glVertexAttribPointer(
			1,
			4,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)color_data_offset );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, r_index_bobj );
	glBindVertexArray( 0 );
	r_used_vaos++;

	/*setup the second vao*/
	pos_data_offset = sizeof(float) * 4 * ( g_num_vtx / 2 );
	color_data_offset += sizeof(float) * 3 * ( g_num_vtx / 2 );

	glGenVertexArrays( 1, &r_vao_ar[1] );
	glBindVertexArray( r_vao_ar[1] );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)pos_data_offset );

	glVertexAttribPointer(
			1,
			4,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)color_data_offset );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, r_index_bobj );
	glBindVertexArray( 0 );
	r_used_vaos++;

	return ERR_OK;
}


/******************************************************************************
func: setup the gl state machine
desc:
 *****************************************************************************/
int rn_t::set_gl_state( void )
{
	/*set up face culling*/
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glFrontFace( GL_CW );

	/*set up depth tests*/
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glDepthFunc( GL_LESS );
	glDepthRange( DR_NEAR, DR_FAR );

	return ERR_OK;
}


/******************************************************************************
func: setup any initial uniforms for the shaders
desc:
 *****************************************************************************/
int rn_t::initialize_shaders( void )
{
	GLuint perspective_mat_unif = 0;

	/*initialze for matrix multiplication*/
	ps_matrix[0] = f_scale;
	ps_matrix[5] = f_scale;
	ps_matrix[10] = ( z_far + z_near ) / ( z_near - z_far );
	ps_matrix[11] = -1.0f;
	ps_matrix[14] = ( 2 * z_far * z_near ) / ( z_near - z_far );

	perspective_mat_unif = glGetUniformLocation( r_sh_program,
			"perspective_matrix" );

	/*send the matrix to the shader*/
	glUseProgram( r_sh_program );
	glUniformMatrix4fv( perspective_mat_unif, 1, GL_FALSE, ps_matrix );
	glUseProgram( 0 );

	return ERR_OK;
}



