#version 330 core 
out vec4 FragColor; 

in vec2 TexCoord;
uniform sampler2D texture0; 
uniform sampler2D texture1; 
uniform float ratio; 
void main() { 
FragColor = mix(texture(texture0, TexCoord), texture(texture1, vec2(TexCoord.x,TexCoord.y)),ratio); 
}
