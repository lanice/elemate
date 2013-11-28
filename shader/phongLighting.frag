#version 330

//Phong Lighting model with two light sources

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 cameraposition, vec3 lightdir, vec3 lightdir2, mat4 light, mat4 light2, vec4 lightambientglobal, mat4 material)
{
    //lightsource uniforms
    
    //ambient intensity    
    vec4 iAmbient[2];
    iAmbient[0] = light[0];
    iAmbient[1] = light2[0];

    //diffuse intensity
    vec4 iDiffuse[2];
    iDiffuse[0] = light[1];
    iDiffuse[1] = light2[1];

    //specular intensity
    vec4 iSpecular[2];
    iSpecular[0]=light[2];
    iSpecular[1]=light2[2];

    //shininess
    float shininess[2];
    shininess[0]= light[3].w;
    shininess[1]= light2[3].w;

    //lightdirection
    vec3 lightdirection[2];
    lightdirection[0]=normalize(lightdir);
    lightdirection[1]=normalize(lightdir2);

    //calculate dist, attenuation coeff
    float dist[2];
    dist[0] = length(lightdir);
    dist[1] = length(lightdir2); 
    vec3 att[2];
    att[0]=light[3].xyz;
    att[1]=light2[3].xyz;
    
    //declaration of attenuation and dotProduct of normal and lightdirection
    //diffuse and specular calculation
    float attenuation[2];
    float nxDir[2];
    vec4 diffuse[2];
    vec4 specular[2];    
    for(int i=0; i<2; i++)
    {
        specular[i]=vec4(0.0);
    }

    //calculate attenuation and dotProduct of normal -> calculate diffuse 
    for(int i=0; i<2; i++)
    {
        attenuation[i]= min(1 /( att[i].x + att[i].y*dist[i] + att[i].z*dist[i]*dist[i]) ,1 );
        nxDir[i] = max(0.0 , dot(n , lightdirection[i]));
        diffuse[i] = iDiffuse[i] * nxDir[i] * attenuation[i];
    }

    //calculate specular
    for(int i=0; i<2; i++)
    {    
        if(nxDir[i] != 0.0)
        {
            vec3 cameraVector = normalize(cameraposition - v_pos);
            vec3 halfVector = normalize(lightdirection[i] + cameraVector);
            float nxHalf = max(0.0 , dot(n , halfVector));
            float specularPower = max(pow(nxHalf , shininess[i]),0.0);
            specular[i] = iSpecular[i] * specularPower * attenuation[i];
        }
    }

    // calculate color from lighting coeff and material coeff

                    //global ambient            //emission
    vec4 color=lightambientglobal * material[0] + material[3];
    for(int i=0; i<2; i++)
    {                            //local ambient            //diffuse                    //specular
        color += color + iAmbient[i] * material[0] + attenuation[i] * (diffuse[i] * material[1] + specular[i] * material[2]);
    }
    return color;
}