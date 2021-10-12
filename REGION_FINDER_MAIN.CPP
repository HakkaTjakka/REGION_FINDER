#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <vector>
#include <map>
#include <limits>
#include <SFML\Graphics.hpp>
#include <SFML\Graphics\Shader.hpp>
#include <SFML\Graphics\Image.hpp>

struct stat stat_buffer;

bool file_exists(const char * filename)
{
    int exist = stat(filename,&stat_buffer);
    if (exist==0) return true;
    return false;
}

struct region {
    int x,z;
};

struct region_size {
    int x_max,x_min,z_max,z_min;
    int x_tot,z_tot;
};

int read_out() {
    struct region_size region_size;
    if (!file_exists("out.txt")) {
        printf("out.txt not found.\n");
        return -1;
    }
    FILE* in=fopen("out.txt","r");
    FILE* out=fopen("marked.txt","w");
    char line[10000];
    int line_count=1;
    if (fgets (line,10000, in)!=NULL ) {
        if ( sscanf(line,"XMIN=%d XMAX=%d XTOT=%d\n",&region_size.x_min, &region_size.x_max, &region_size.x_tot) == 3 ) {
            printf("XMIN=%d XMAX=%d XTOT=%d\n", region_size.x_min, region_size.x_max, region_size.x_tot);
            fprintf(out,"XMIN=%d XMAX=%d XTOT=%d\n", region_size.x_min, region_size.x_max, region_size.x_tot);
        } else {
            printf("Error reading line %d from out.txt: %s\n",line_count,line);
            return -1;
        }
        line_count++;
    } else {
        printf("Error reading line %d from out.txt\n",line_count);
        return -1;
    }
    if (fgets (line,10000, in)!=NULL ) {
        if ( sscanf(line,"ZMIN=%d ZMAX=%d ZTOT=%d\n",&region_size.z_min, &region_size.z_max, &region_size.z_tot) == 3 ) {
            printf("ZMIN=%d ZMAX=%d ZTOT=%d\n", region_size.z_min, region_size.z_max, region_size.z_tot);
            fprintf(out,"ZMIN=%d ZMAX=%d ZTOT=%d\n", region_size.z_min, region_size.z_max, region_size.z_tot);
        } else {
            printf("Error reading line %d from out.txt: %s\n",line_count,line);
            return -1;
        }
        line_count++;
    } else {
        printf("Error reading line %d from out.txt\n",line_count);
        return -1;
    }

    int x_max = region_size.x_max;
    int x_min = region_size.x_min;
    int z_max = region_size.z_max;
    int z_min = region_size.z_min;
    int x_tot = region_size.x_tot;
    int z_tot = region_size.z_tot;

    printf("\nFilling array\n");
    printf("%d < x < %d = %d\n", x_min, x_max, x_tot);
    printf("%d < z < %d = %d\n", z_min, z_max, z_tot);

    int x,z;
    char regions[z_tot][x_tot];
    line_count=0;
    for (z=0; z<z_tot; z++) {
        line_count++;
        if (fgets (line,10000, in)!=NULL ) {
            int line_len=strlen(line);
            if (line_len!=x_tot+1) {
                printf("Warning, line %d length!=%d strlen(line)-1=%d\n", line_count, x_tot, line_len-1);
            }
            for (x=0; x<line_len-1; x++) {
                regions[z][x]=line[x];
                if ( ! (line[x]=='.' || line[x]=='*') ) {

                    fprintf(out,"r.%d.%d (%c)", x+x_min, z+z_min, line[x]);

                    char test_file[1000];
                    bool mca_exists, vox_exists;

                    sprintf(test_file,"E:/SAVES/TEST/REGION/DONE0/r.%d.%d.mca",x+x_min,z+z_min);
                    mca_exists=file_exists(test_file);

                    sprintf(test_file,"G:/DONE/r.%d.%d.vox",x+x_min,z+z_min);
                    vox_exists=file_exists(test_file);

                    if (mca_exists) fprintf(out, " mca");
                    else fprintf(out, "  x ");

                    if (vox_exists) fprintf(out, " vox");
                    else fprintf(out, "  x ");
                    fprintf(out, "\n");
                }
            }
        } else {
            printf("Error reading line %d from out.txt\n",line_count);
            return -1;
        }
    }
    fclose(in);
    fclose(out);

    int count=0;

    return 0;
}

int check_regions(  std::map<long long int, region> &regions_mapped,
                    std::map<long long int, region> &voxels_mapped,
                    struct region_size &region_size,
                    char* directory_voxels
    ) {
    struct region one_region;
    int x,z;
//    std::map<long long int, region>::iterator it_regions_mapped;
    int x_max = region_size.x_max+1;
    int x_min = region_size.x_min-1;
    int z_max = region_size.z_max+1;
    int z_min = region_size.z_min-1;
    int x_tot = x_max-x_min+1;
    int z_tot = z_max-z_min+1;

    printf("\nFilling array\n");
    printf("%d < x < %d = %d\n", x_min, x_max, x_tot);
    printf("%d < z < %d = %d\n", z_min, z_max, z_tot);

    sf::Image m_image;
    m_image.create(x_tot, z_tot, sf::Color(0,0,0,0));

    char regions[z_tot][x_tot];
    for (z=0; z<z_tot; z++) {
        for (x=0; x<x_tot; x++) {
            regions[z][x]=0;
        }
    }

    int count=0;
    std::map<long long int, region>::iterator it;

    it = regions_mapped.begin();

    sf::Color pixel;
    while(it != regions_mapped.end()) {
//    for (auto v : regions_mapped) {
//        x = v.second.x;
//        z = v.second.z;
        x = it->second.x;
        z = it->second.z;
        if ( (z-z_min>=0) && (z-z_min<z_tot) && (x-x_min>=0) && (x-x_min<x_tot) ) {
            regions[z-z_min][x-x_min]=1;
            count++;
        } else {
            printf("\nout of range: r.%d.%d.mca  \r",x,z);
        }
//        for (int zz=-1; zz<=1; zz++) {
//            for (int xx=-1; xx<=1; xx++) {
//                if (!(xx==0 && zz==0)) {
//                    // find surrounding empty spot(s)
//                    if (regions_mapped.find((z+zz)*1000000+x+xx) == regions_mapped.end()) { // empty spot
//                        count++;
//                    }
//                }
//            }
//        }
//        printf("r.%d.%d.mca %d   \r",x,z,count);
        if (!(count%1000)) printf("r.%d.%d.mca %d   \r",x,z,count);
        it++;
    }
    printf("r.%d.%d.mca %d   \n",x,z,count);

    count=0;

    it = voxels_mapped.begin();
    while(it != voxels_mapped.end()) {
//    for (auto v : voxels_mapped) {
//        x = v.second.x;
//        z = v.second.z;
        x = it->second.x;
        z = it->second.z;
        if ( (z-z_min>=0) && (z-z_min<z_tot) && (x-x_min>=0) && (x-x_min<x_tot) ) {
            regions[z-z_min][x-x_min]+=2;

//            printf("r.%d.%d.vox %d   \r",x,z,count);
            count++;
        } else {
            printf("\nout of range: r.%d.%d.vox  \r",x,z);
        }
        if (!(count%1000)) printf("r.%d.%d.vox %d   \r",x,z,count);
//        printf("r.%d.%d.mca %d   \r",x,z,count);
        it++;
    }
    printf("r.%d.%d.vox %d   \n",x,z,count);

    FILE* out;      out=fopen("out.txt","w");
    FILE* wrong;    wrong=fopen("FAIL.TXT","w");

    fprintf(out,"XMIN=%d XMAX=%d XTOT=%d\n", x_min, x_max, x_tot);
    fprintf(out,"ZMIN=%d ZMAX=%d ZTOT=%d\n", z_min, z_max, z_tot);

    count=0;


    for (z=0; z<z_tot; z++) {
//        printf("z=%7d ",z+z_min);
        printf("r.%d.%d %d   \r",x+x_min,z+z_min,count);
        for (x=0; x<x_tot; x++) {
            unsigned char shade500=(int(abs(x+x_min)/500) + int(abs(z+z_min)/500)      )%2 ;
            unsigned char shade50= (int(abs(x+x_min)/ 50) + int(abs(z+z_min)/ 50) + shade500     )%2 ;
            unsigned char shade5=  (int(abs(x+x_min)/  5) + int(abs(z+z_min)/  5) + shade50 )%2 ;
            unsigned char shade1=  (int(abs(x+x_min)    ) + int(abs(z+z_min)    ) + shade5    )%2 ;
            unsigned char shade=(shade1+shade5+shade50+shade500)*20;


            count++;
            int n=regions[z][x];
            //printf("%d",n);
            if (n==1) {
                fputc('*',out);
//                m_image.setPixel(x,z,sf::Color(shade, 255-shade, shade, 255));
                m_image.setPixel(x,z,sf::Color(shade, 255-shade, shade, 255));
            }
            else if (n==2) {
                char filename[1000];
                sprintf(filename,"%s/r.%d.%d.vox",directory_voxels,x+x_min,z+z_min);
                if (file_exists(filename)) {
                    fprintf(wrong,"r.%d.%d.vox\n",x+x_min,z+z_min);
                    fputc('V',out);
                } else {
                    fprintf(wrong,"r.%d.%d.vox (doesn exist??)\n",x+x_min,z+z_min);
                    fputc('V',out);
                }
                m_image.setPixel(x,z,sf::Color(255-shade, shade, shade, 255));
            }
            else if (n==3) {
                fputc('*',out);
                m_image.setPixel(x,z,sf::Color(128-shade/2, 255-shade, shade, 255));
            }
            else {
                m_image.setPixel(x,z,sf::Color(255-shade, 255-shade, 255-shade, 255));
                fputc('.',out);
            }
        }
        //printf("\n");
        fprintf(out,"\n");
    }
    printf("r.%d.%d %d (%d x %d = %d)  \n",x+x_min,z+z_min,count, x_tot, z_tot, x_tot*z_tot);

    m_image.saveToFile("region.png");
    fclose(out);
    fclose(wrong);
    printf("\n");

    sf::RenderWindow sfmlWin(sf::VideoMode(x_tot, z_tot), "PACMAN", sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close);

//    sf::RenderWindow sfmlWin(sf::VideoMode(x_tot, z_tot), "Hello World SFML Window", sf::Style::Resize);
    sf::Texture m_texture;
    m_texture.loadFromImage(m_image);
    sf::Sprite m_sprite=sf::Sprite(m_texture);
    float aspect_ratio=x_tot/z_tot;
    sfmlWin.clear();
    sfmlWin.draw(m_sprite);
    sfmlWin.display();
    sfmlWin.setVerticalSyncEnabled(true);

    sf::Shader m_shader;
    m_shader.loadFromFile("template.vert","template.frag");
    m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));

    sf::RenderTexture m_render_texture;
    m_render_texture.create(m_texture.getSize().x,m_texture.getSize().y);
    sf::Sprite m_sprite2=sf::Sprite(m_render_texture.getTexture());

    while (sfmlWin.isOpen()) {
        sf::sleep(sf::seconds(0.1));
        sf::Event e;
        while (sfmlWin.pollEvent(e)) {
            switch (e.type) {
            case sf::Event::EventType::KeyPressed:
                switch(e.key.code) {
                case sf::Keyboard::I:
                    m_shader.setUniform("iResolution", sf::Vector2f(m_texture.getSize()));

                    m_render_texture.draw(m_sprite,&m_shader);
                    m_render_texture.display();
                    m_render_texture.getTexture().copyToImage().saveToFile("output.png");

                    m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));
                    break;
                }
                break;
            case sf::Event::EventType::Closed:
                sfmlWin.close();
                break;
            case sf::Event::EventType::Resized:
                sfmlWin.setSize( sf::Vector2u(sfmlWin.getSize().x,sfmlWin.getSize().x*aspect_ratio) );
                m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));
                break;
            }
        }
        sfmlWin.draw(m_sprite,&m_shader);
        sfmlWin.display();
        sfmlWin.clear();
    }

    return 0;
}

int find_regions(char* directory, std::map<long long int, region> &regions_mapped, struct region_size &region_size) {
    DIR* m_dir;
    struct dirent *m_dirent;

    printf("\nReading %s/r.*.*.mca\n",directory);
    m_dir = opendir(directory);
    struct region one_region;
//    std::map<long long int, region>::iterator it_regions_mapped;

//    long long totalbytes=0;
    int x_max=-std::numeric_limits<int>::max();
    int x_min=std::numeric_limits<int>::max();
    int z_max=-std::numeric_limits<int>::max();
    int z_min=std::numeric_limits<int>::max();


    int count=0;
    char* name;
    while ((m_dirent = readdir(m_dir)) != NULL) {
        name=m_dirent->d_name;
        if ((strstr(name, ".mca")) != NULL) {
            if ( sscanf(name,"r.%d.%d.mca",&one_region.x,&one_region.z) == 2 ) {
                if (!(count%1000)) printf("%s %d   \r",name,count);
                count++;
                regions_mapped.insert(std::make_pair((long long)one_region.z*1000000+(long long)one_region.x, one_region));
                x_max=std::max( x_max , one_region.x);
                x_min=std::min( x_min , one_region.x);
                z_max=std::max( z_max , one_region.z);
                z_min=std::min( z_min , one_region.z);
            } else {
                printf("Wrong filename format: %s\n",name);
            }

//            char fullname[1000];
//            strcpy(fullname,directory);
//            strcat(fullname,"/");
//            strcat(fullname,name);
//            file_exists(fullname);
//            totalbytes+=stat_buffer.st_size;
//            printf("r.%d.%d.mca %s %d\n",one_region.x,one_region.z,fullname,stat_buffer.st_size);
//            printf("%s %d bytes total=%lld bytes        \r",fullname,stat_buffer.st_size,totalbytes);

//            printf("%s\r",name);

//            it_regions_mapped=regions_mapped.find(one_region.z*1000000+one_region.x);
//            if (it_regions_mapped != regions_mapped.end()) {
//                printf("%s r.%d.%d %lld\n",
//                    name,
//                    it_regions_mapped->second.x,
//                    it_regions_mapped->second.z,
//                    it_regions_mapped->first);
//            } else {
//                printf("Error\n");
//            }
        }
    }
    printf("%s %d   \r",name,count++);

    closedir(m_dir);
    region_size.x_max=x_max;
    region_size.x_min=x_min;
    region_size.z_max=z_max;
    region_size.z_min=z_min;
    return regions_mapped.size();
}

int find_voxels(char* directory, std::map<long long int, region> &regions_mapped, struct region_size &region_size) {
    DIR* m_dir;
    struct dirent *m_dirent;

    printf("\nReading %s/r.*.*.vox\n",directory);
    m_dir = opendir(directory);
    struct region one_region;

    int x_max=region_size.x_max;
    int x_min=region_size.x_min;
    int z_max=region_size.z_max;
    int z_min=region_size.z_min;

    int count=0;
    char* name;
    while ((m_dirent = readdir(m_dir)) != NULL) {
        name=m_dirent->d_name;
        if ( (strstr(name, ".vox")) != NULL ) {
            if ( sscanf(name,"r.%d.%d.vox",&one_region.x,&one_region.z) == 2 ) {
                if (!(count%1000)) printf("%s %d   \r",name,count);
                count++;
                regions_mapped.insert( std::make_pair((long long)one_region.z*1000000+(long long)one_region.x, one_region) );
                x_max=std::max( x_max , one_region.x );
                x_min=std::min( x_min , one_region.x );
                z_max=std::max( z_max , one_region.z );
                z_min=std::min( z_min , one_region.z );
            } else {
                printf("Wrong filename format: %s\n",name);
            }
        }
    }
    printf("%s %d   \n",name,count++);
    closedir(m_dir);
    region_size.x_max=x_max;
    region_size.x_min=x_min;
    region_size.z_max=z_max;
    region_size.z_min=z_min;
    return regions_mapped.size();
}

int quick_window(char* m_image_filename) {

    sf::Image m_image;

    m_image.loadFromFile(m_image_filename);

    sf::RenderWindow sfmlWin2(sf::VideoMode(m_image.getSize().x,m_image.getSize().y), "PACMAN2", sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close);

    sf::RenderWindow sfmlWin(sf::VideoMode(m_image.getSize().x,m_image.getSize().y), "PACMAN", sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close);

//    sf::RenderWindow sfmlWin(sf::VideoMode(x_tot, z_tot), "Hello World SFML Window", sf::Style::Resize);
    sf::Texture m_texture;

    m_texture.loadFromImage(m_image);
    sf::Sprite m_sprite=sf::Sprite(m_texture);
    float aspect_ratio=m_image.getSize().x/m_image.getSize().y;
    sfmlWin.clear();
    sfmlWin.draw(m_sprite);
    sfmlWin.display();
    sfmlWin.setVerticalSyncEnabled(true);

    sf::Shader m_shader;

//    m_shader.loadFromFile("template.vert","template.frag");
//    m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));

    sf::Texture m_texture_font;
    m_texture_font.loadFromFile("HE2vH.png");

    m_shader.loadFromFile("text.vert","text.frag");
    m_shader.setUniform("fxs", (float)16.0/(float)sfmlWin.getSize().x);
    m_shader.setUniform("fys", (float)16.0/(float)sfmlWin.getSize().y);
    m_shader.setUniform("txr_font", m_texture_font);

//    m_shader.loadFromFile("template.vert","template.frag");
//    m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));

    m_image.setPixel(0,0,sf::Color('P','a','c','m'));
    m_image.setPixel(1,0,sf::Color('a','n',' ',' '));

    m_texture.loadFromImage(m_image);

    sf::RenderTexture m_render_texture;
    m_render_texture.create(m_texture.getSize().x,m_texture.getSize().y);
    sf::Sprite m_sprite2=sf::Sprite(m_render_texture.getTexture());

    m_shader.setUniform("iResolution", sf::Vector2f(m_texture.getSize()));
    m_shader.setUniform("fxs", (float)16.0/(float)m_texture.getSize().x);
    m_shader.setUniform("fys", (float)16.0/(float)m_texture.getSize().y);

    m_render_texture.draw(m_sprite,&m_shader);
    m_render_texture.display();

    while (sfmlWin.isOpen()) {
        sf::sleep(sf::seconds(0.1));
        sf::Event e;
        while (sfmlWin.pollEvent(e)) {
            switch (e.type) {
            case sf::Event::EventType::KeyPressed:
                switch(e.key.code) {
                case sf::Keyboard::I:
                    sf::Color pixel=sf::Color(m_image.getPixel(0,0));
                    printf("Pixel(r,g,b,a)=(%d,%d,%d,%d)(%c%c%c%c)\n",
                           pixel.r,pixel.g,pixel.b,pixel.a,
                           pixel.r,pixel.g,pixel.b,pixel.a
                           );
                    pixel=sf::Color(m_image.getPixel(1,0));
                    printf("Pixel(r,g,b,a)=(%d,%d,%d,%d)(%c%c%c%c)\n",
                           pixel.r,pixel.g,pixel.b,pixel.a,
                           pixel.r,pixel.g,pixel.b,pixel.a
                           );

                    m_shader.setUniform("iResolution", sf::Vector2f(m_texture.getSize()));
                    m_shader.setUniform("fxs", (float)16.0/(float)m_texture.getSize().x);
                    m_shader.setUniform("fys", (float)16.0/(float)m_texture.getSize().y);

                    m_render_texture.draw(m_sprite,&m_shader);
                    m_render_texture.display();
                    sfmlWin2.draw(m_sprite2);
                    m_image = m_render_texture.getTexture().copyToImage();
                    m_image.saveToFile("output.png");

                    m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));
                    m_shader.setUniform("fxs", (float)16.0/(float)sfmlWin.getSize().x);
                    m_shader.setUniform("fys", (float)16.0/(float)sfmlWin.getSize().y);

                    pixel=sf::Color(m_image.getPixel(0,0));
                    printf("Pixel(r,g,b,a)=(%d,%d,%d,%d)(%c%c%c%c)\n",
                           pixel.r,pixel.g,pixel.b,pixel.a,
                           pixel.r,pixel.g,pixel.b,pixel.a
                           );
                    pixel=sf::Color(m_image.getPixel(1,0));
                    printf("Pixel(r,g,b,a)=(%d,%d,%d,%d)(%c%c%c%c)\n",
                           pixel.r,pixel.g,pixel.b,pixel.a,
                           pixel.r,pixel.g,pixel.b,pixel.a
                           );

//                        sf::Color pixel = m_render_texture.getTexture().copyToImage().getPixel(0,0);
//                        m_image.setPixel(0,0,sf::Color( pixel.r, pixel.g, pixel.b, pixel.a ));
//                        m_image.setPixel(1,0,sf::Color('J','e','u','j'));
                    m_texture.loadFromImage(m_image);
                    sfmlWin2.display();

                    break;
                }
                break;
            case sf::Event::EventType::Closed:
                sfmlWin.close();
                break;
            case sf::Event::EventType::Resized:
                sfmlWin.setSize( sf::Vector2u(sfmlWin.getSize().x,sfmlWin.getSize().x*aspect_ratio) );
                m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));
                m_shader.setUniform("fxs", (float)16.0/(float)sfmlWin.getSize().x);
                m_shader.setUniform("fys", (float)16.0/(float)sfmlWin.getSize().y);
//                m_shader.setUniform("fxs", (float)1.0/);
//                m_shader.setUniform("fys", (float)1.0/);
                break;
            }
        }
        sfmlWin.draw(m_sprite,&m_shader);
        sfmlWin.display();

        sfmlWin.clear();
    }

    return 0;
}

int main(int argc, char ** argv) {

//    for (int n=0; n<argc; n++) printf("argv[%d]=%s\n",n,argv[n]);
//    getchar();

    if (!(argc>1)) {
        printf("Usage: %s out.txt | <region_dir> [<voxels_done_dir>]\n",argv[0]);
        return -1;
    }

    if (strcmp(argv[1],"test")==0) {
        quick_window("region.png");
        return 0;
    }

    if (!file_exists(argv[1])) {
        printf("%s does not exist\n");
        return -1;
    }

    if (argc>2) {
        if (!file_exists(argv[2])) {
            printf("%s does not exist\n");
            return -1;
        }
    }

    if (strcmp(argv[1],"out.txt")==0) {
        int ret=read_out();
        return ret;
    }

    std::map<long long int, region> regions_mapped;
    std::map<long long int, region> voxels_mapped;

    struct region_size region_size;

    int ret = find_regions(argv[1], regions_mapped, region_size);
    printf("\nRegion files found: %d\n", ret);

    int x_max = region_size.x_max;
    int x_min = region_size.x_min;
    int z_max = region_size.z_max;
    int z_min = region_size.z_min;
    int x_tot = x_max-x_min+1;
    int z_tot = z_max-z_min+1;
    printf("%d < x < %d = %d\n", x_min, x_max, x_tot);
    printf("%d < z < %d = %d\n", z_min, z_max, z_tot);

    if (argc>2) {
        ret = find_voxels(argv[2], voxels_mapped, region_size);
        printf("\nVoxel files found: %d\n", ret);
        x_max = region_size.x_max;
        x_min = region_size.x_min;
        z_max = region_size.z_max;
        z_min = region_size.z_min;
        x_tot = x_max-x_min+1;
        z_tot = z_max-z_min+1;
        printf("%d < x < %d = %d\n", x_min, x_max, x_tot);
        printf("%d < z < %d = %d\n", z_min, z_max, z_tot);
        check_regions(regions_mapped, voxels_mapped, region_size, argv[2] );
    } else {
        check_regions(regions_mapped, voxels_mapped, region_size, "" );
    }


//    check_regions(voxels_mapped, regions_mapped, region_size);
//    for (auto v : regions_mapped) {
//        printf("test: r.%d.%d.mca\n", v.second.x, v.second.z);
//    }
    return 0;
}

 /*
 int main() {
     struct stat buf;
     stat("file",&buf);
               ...
     cout << st_dev << endl;
     cout << st_ino << endl;
     cout << st_mode << endl;
     cout << st_nlink << endl;
     cout << st_uid << endl;
     cout << st_gid << endl;
     cout << st_rdev << endl;
     cout << st_size << endl;
     cout << st_blksize << endl;
     cout << st_blocks << endl;
     cout << st_atime << endl;
     cout << st_mtime << endl;
     cout << st_ctime << endl;
     ...
 }
 */
