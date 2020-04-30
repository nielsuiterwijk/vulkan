#pragma once

void to_json( json& j, const ShaderInput& shaderInput )
{
	ASSERT_FAIL( "to_json is not implemented" );
	//j = json{ { "type", shaderInput.type },{ "name", shaderInput.name },{ "location", shaderInput.location } };
}

void from_json( const json& json, ShaderInput& shaderInput )
{
#if DEBUG
	json.at( "type" ).get_to( shaderInput.type );
	json.at( "name" ).get_to( shaderInput.name );
#endif
	json.at( "location" ).get_to( shaderInput.location );

	shaderInput.typeFormat = Vulkan::GetFormatFromType( json.at( "type" ) );
	shaderInput.offset = Vertex::GetOffsetFromLocation( shaderInput.location ); //TODO: This is hardcoded for a specific layout, would need to be made dynamic
}
