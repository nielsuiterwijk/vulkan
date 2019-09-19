#pragma once

void to_json( json& j, const ShaderInput& shaderInput )
{
	throw std::runtime_error( "to_json is not implemented" );
	//j = json{ { "type", shaderInput.type },{ "name", shaderInput.name },{ "location", shaderInput.location } };
}

void from_json( const json& j, ShaderInput& shaderInput )
{
#if DEBUG
	j.at( "type" ).get_to( shaderInput.type );
	j.at( "name" ).get_to( shaderInput.name );
#endif
	j.at( "location" ).get_to( shaderInput.location );

	shaderInput.typeFormat = Vulkan::GetFormatFromType( j.at( "type" ) );
	shaderInput.offset = Vertex::GetOffsetFromLocation( shaderInput.location );
}
