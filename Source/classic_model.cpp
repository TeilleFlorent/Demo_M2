#include "classic_model.hpp"




    Mesh::Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, float shini_mesh, glm::vec2 max_tex_coord, glm::vec2 min_tex_coord)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->shininess = shini_mesh;

        this->_max_tex_coord = max_tex_coord;

        this->_min_tex_coord = min_tex_coord;

        this->setupMesh();
    }


    // render du mesh
    void Mesh::Draw(Shader shader, int id) 
    {
        // var pour bind la bonne tex
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;
        GLuint normalNr = 1;
        GLuint heightNr = 1;


        for(GLuint i = 0; i < this->textures.size(); i++)
        {
            
            //std::cout << "ite = " << i << std::endl;

            glActiveTexture(GL_TEXTURE0 + i); // active la texture qu'il faut

            stringstream ss;
            string number;
            string name = this->textures[i].type;

            //std::cout << "tex = " << name << ", i = " << i << std::endl;

            if(name == "texture_diffuse")
                ss << diffuseNr++; // bricolage pour generer le string pour l'uniform avec le bon numero de texture
            if(name == "texture_specular")
                ss << specularNr++; // bricolage pour generer le string pour l'uniform avec le bon numero de texture
            if(name == "texture_normal")
                ss << normalNr++; // bricolage pour generer le string pour l'uniform avec le bon numero de texture
            if(name == "texture_height")
                ss << heightNr++; // bricolage pour generer le string pour l'uniform avec le bon numero de texture
            

            number = ss.str(); 

            /*if(id == 0){
                std::cout << "TEST 2 = " << this->_max_tex_coord.x << " , " << this->_max_tex_coord.y << " || " << this->_min_tex_coord.x << ", " << this->_min_tex_coord.y << std::endl;
            }*/

            // du coup envoi le string correct correspondant a la texture traité
            //std::cout << "uniform sampler name = " << (name + number) << std::endl;

            glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, this->textures[i].id); // bind la tex qui correspond au string generer et envoyer au juste avant
        }
       

        // draw le mesh
        glBindVertexArray(this->VAO);
        
      
      /*  if(id == 5.0){
           glActiveTexture(GL_TEXTURE7);
           glBindTexture(GL_TEXTURE_2D, this->textures[4].id);
        }*/

        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, this->indices.size());
        glBindVertexArray(0);


        // dé bind toute les tex utilisé pour le draw
        for (GLuint i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }



    // initialise tout les buffer etc
    void Mesh::setupMesh()
    {
        
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindVertexArray(this->VAO);
       
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        // Vertex Positions
        glEnableVertexAttribArray(0);   
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        // Vertex Normals
        glEnableVertexAttribArray(1);   
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
        // Vertex Texture Coords
        glEnableVertexAttribArray(2);   
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
        
        // Vertex Tangent
        glEnableVertexAttribArray(3);   
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Tangent));

        // Vertex Bi Tangent
        glEnableVertexAttribArray(4);   
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, BiTangent));


        glBindVertexArray(0);
    }


    //////////////////////////////////


    Model::Model(){

    }

    // draw tout les meshes du model
    void Model::Draw(Shader shader, glm::mat4 modelview2,bool test)
    {

        for(GLuint i = 0; i < this->meshes.size(); i++){

            //std::cout << "test = " << this->meshes.size() << std::endl;
                     
              if(test && this->model_id == 2){
                  
                  if(i <= 7)
                      this->meshes[i].Draw(shader, this->model_id);

              }else{
            
                  this->meshes[i].Draw(shader, this->model_id);
              }
        }
    }
    

    //void Load_Model(GLchar* path){
    void Model::Load_Model(string path, int id){

        this->model_id = id;
        this->loadModel(path);

    }

    void Model::Print_info_model(){

        float res = 0;
        cout << "\nCLASSIC MODEL:\n" << "nbMeshes = " << meshes.size() << endl;
        
        for(unsigned int i = 0; i < meshes.size(); i++){
            cout << "mesh " << i << ", nbVertices = " << meshes[i].vertices.size() << endl;
            //cout << "nbIndices = " << meshes[i].indices.size() << endl;
            res+=meshes[i].vertices.size();

        }

        NumVertices = res;
        cout << "nb_vertices_total = " << res << "\n" << endl;
        

    }

    
    void Model::loadModel(string path)
    {

        /*aiString bla;   
        importer.GetExtensionList(bla);
        cout << bla.C_Str() << endl; // print tous les format supporté
*/

        //cout << "id = " << model_id << endl;
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        /*if(this->model_id == 5){
            std::cout << "TEST" << std::endl;
        }*/


        if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR ASSIMP (dans la fonction loadModel) " << importer.GetErrorString() << endl;
            return;
        }

        this->directory = path.substr(0, path.find_last_of('/'));

        this->processNode(scene->mRootNode, scene, 0);
    }


    void Model::processNode(aiNode* node, const aiScene* scene, int num_mesh)
    {


        for(GLuint i = 0; i < node->mNumMeshes; i++)
        {

            //cout << "num mesh = " << num_mesh << endl;

            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 

            //cout << "mesh_name = " << mesh->mName.data << endl;

            if(mesh->mName != aiString((std::string("Paladin_J_Nordstrom_Sword"))) && mesh->mName != aiString((std::string("Paladin_J_Nordstrom_Shield"))))
                this->meshes.push_back(this->processMesh(mesh, scene, num_mesh));  


        }
       
        for(GLuint i = 0; i < node->mNumChildren; i++)
        {

            this->processNode(node->mChildren[i], scene, i);
        }

    }

    Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene, int num_mesh)
    {

       
        vector<Vertex> vertices;
        vector<GLuint> indices;
        vector<Texture> textures;
        glm::vec2 max_tex_coord;
        max_tex_coord = glm::vec2(0.0,0.0);
        glm::vec2 min_tex_coord;
        min_tex_coord = glm::vec2(999.0,999.0);



        for(GLuint i = 0; i < mesh->mNumVertices; i++)
        {

            Vertex vertex;
            glm::vec3 vector; 
            // Position
            
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
        
            //std::cout << "x = " << vector.x << ", y = " << vector.y << std::endl;
                
            vertex.Position = vector;
            // Normal
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            //std::cout << "x = " << vector.x << ", y = " << vector.y << ", z = " << vector.z << std::endl;
                
            vertex.Normal = vector;



            // Texture Coord
            if(mesh->mTextureCoords[0]) // verifie si il y a des tex coord
            {
                //printf("IL Y A DES TEX COORD\n");

                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                //std::cout << "x = " << vec.x << ", y = " << vec.y << std::endl;
                vertex.TexCoords = vec;

                // get max tex coord
                if(vec.x > max_tex_coord.x){
                    max_tex_coord.x = vec.x;
                }
                if(vec.y > max_tex_coord.y){
                    max_tex_coord.y = vec.y;
                }

                // get min tex coord
                if(vec.x < min_tex_coord.x){
                    min_tex_coord.x = vec.x;
                }
                if(vec.y < min_tex_coord.y){
                    min_tex_coord.y = vec.y;
                }


            }
            else{
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                printf("PAS DE TEX COORD\n");
            }



            // ADD TANGENT
        /*    vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;

            vertex.Tangent = vector; 

            // ADD BI TANGENT
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;

            vertex.BiTangent = vector; */




            //std::cout << "test = " << vertex.Tangent.x << vertex.Tangent.y << vertex.Tangent.z << std::endl; 

            vertices.push_back(vertex);
        }

        //std::cout << "TEST 1 = " << max_tex_coord.x << " , " << max_tex_coord.y << " || " << min_tex_coord.x << ", " << min_tex_coord.y << std::endl;

        // ADD Bi Tangente
        for(GLuint i = 0; i < vertices.size(); i+=3){

            //std::cout << "test = " << i << std::endl;

           // Raccourcis pour les sommets
           glm::vec3 v0,v1,v2;
     
           v0 = vertices[i].Position;
           v1 = vertices[i+1].Position;
           v2 = vertices[i+2].Position; 
       

     

            // Raccourcis pour les UV
           glm::vec2 uv0,uv1,uv2;

            uv0 = vertices[i].TexCoords;
            uv1 = vertices[i+1].TexCoords;
            uv2 = vertices[i+2].TexCoords;


            // Côtés du triangle : delta des positions
            glm::vec3 deltaPos1 = v1-v0; 
            glm::vec3 deltaPos2 = v2-v0; 

            // delta UV
            glm::vec2 deltaUV1 = uv1-uv0; 
            glm::vec2 deltaUV2 = uv2-uv0;

            /////////////
            
            float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x); 
            glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r; 
            glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

            vertices[i].Tangent = tangent; 
            vertices[i+1].Tangent = tangent;
            vertices[i+2].Tangent = tangent;

            vertices[i].BiTangent = bitangent; 
            vertices[i+1].BiTangent = bitangent;
            vertices[i+2].BiTangent = bitangent;



        }

        for(GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }


        //std::cout << "test1 = " << scene->HasTextures() << std::endl;

        // concerne les material texture
        if(/*mesh->mMaterialIndex >= 0*/ true)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            
            //std::cout << "test1 = " << scene->HasTextures() << std::endl;


            // Diffuse maps
            vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", num_mesh);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            /*if(model_id != 0){
            // Specular maps
                vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", num_mesh);
                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
           // Specular maps
                vector<Texture> normalMaps = this->loadMaterialTextures(material, aiTextureType_HEIGHT , "texture_normal", num_mesh);
                textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            }*/
        }
          

        //std::cout << "test = " << num_mesh << std::endl;
        return Mesh(vertices, indices, textures, 1.0, max_tex_coord, min_tex_coord);
    }

    
    vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, int num_mesh)
    {


        vector<Texture> textures;

         // FOR TABLE1 ONLY
         if(this -> model_id == 0){

            GLboolean skip = false;

            string temp1;
            
            aiString str;
            

            if(!skip){
               // albedo
               temp1 = "albedo.png";            
               str.Set(temp1);

               Texture texture;
               texture.id = TextureFromFile(str.C_Str() , this->directory);
               texture.path = str;
               temp1 = "texture_diffuse";
               texture.type = temp1;
               textures.push_back(texture);
               this->textures_loaded.push_back(texture);  
            
                // normal
                 temp1 = "normal.png";            
                 str.Set(temp1);
            
                 texture.id = TextureFromFile(str.C_Str() , this->directory);
                 texture.path = str;
                 temp1 = "texture_normal";
                 texture.type = temp1;
                 textures.push_back(texture);
                 this->textures_loaded.push_back(texture); 

                 // height
                 temp1 = "height.png";            
                 str.Set(temp1);

                 texture.id = TextureFromFile(str.C_Str() , this->directory);
                 texture.path = str;
                 temp1 = "texture_height";
                 texture.type = temp1;
                 textures.push_back(texture);
                 this->textures_loaded.push_back(texture);  
   
           }

           return textures;
       }


    
        for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
        {

            aiString str;
            mat->GetTexture(type, i, &str);
            
            string test_path = str.data;
              
            //std::cout << "str = " << str.data << std::endl;


            GLboolean skip = false;
            for(GLuint j = 0; j < textures_loaded.size(); j++)
            {
                if(textures_loaded[j].path == str)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; 
                    break;
                }
            }
            if(!skip)
            {   
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                this->textures_loaded.push_back(texture);  
            }


        }

      /*  if(this->model_id == 2){
            std::cout << "nb tex = " << textures.size() << std::endl;
        }*/
        return textures;
    }


    GLint Model::TextureFromFile(const char* path, string directory)
    {

    float aniso;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso); // get la valeur pour l'aniso

    SDL_Surface * t = NULL;

    string filename = string(path);
    filename = directory + '/' + filename;
    GLuint textureID;
    glGenTextures(1, &textureID);
    t = IMG_Load(filename.c_str());


   /* std::cout << "test1 = " << path << std::endl;
    std::cout << "test2 = " << directory << std::endl;     
    std::cout << "test3 = " << filename << std::endl;   */  
    

/*    if(!t)
        printf("image null\n");*/

    glBindTexture(GL_TEXTURE_2D, textureID);
    
    if(t->format->format == SDL_PIXELFORMAT_RGB332
        || t->format->format == SDL_PIXELFORMAT_RGB444
        || t->format->format == SDL_PIXELFORMAT_RGB555
        || t->format->format == SDL_PIXELFORMAT_RGB565
        || t->format->format == SDL_PIXELFORMAT_RGB24
        || t->format->format == SDL_PIXELFORMAT_RGB888
        //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
        || t->format->format == SDL_PIXELFORMAT_RGB565
        || t->format->format == SDL_PIXELFORMAT_BGR555
        || t->format->format == SDL_PIXELFORMAT_BGR565
        || t->format->format == SDL_PIXELFORMAT_BGR24
        || t->format->format == SDL_PIXELFORMAT_BGR888){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
    }else{
    
    if(t->format->format == SDL_PIXELFORMAT_RGBA4444
        || t->format->format == SDL_PIXELFORMAT_RGBA5551
        || t->format->format == SDL_PIXELFORMAT_ARGB4444
        || t->format->format == SDL_PIXELFORMAT_ABGR4444
        || t->format->format == SDL_PIXELFORMAT_BGRA4444
        || t->format->format == SDL_PIXELFORMAT_ABGR1555
        || t->format->format == SDL_PIXELFORMAT_BGRA5551
        || t->format->format == SDL_PIXELFORMAT_ARGB8888
        || t->format->format == SDL_PIXELFORMAT_ABGR8888
        || t->format->format == SDL_PIXELFORMAT_BGRA8888
        //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
        || t->format->format == SDL_PIXELFORMAT_RGBA8888){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
    }else{ glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels); }
}
    
    
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB,GL_UNSIGNED_BYTE, t->pixels);
    
    glGenerateMipmap(GL_TEXTURE_2D);    

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(t);
    return textureID;
    }
