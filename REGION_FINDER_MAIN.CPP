#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <io.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <vector>
#include <map>
#include <limits>
#include <SFML\Graphics.hpp>
#include <SFML\Graphics\Shader.hpp>
#include <SFML\Graphics\Image.hpp>
#include <iostream>
#include <errno.h>

using namespace std;

void make_border(sf::Image m_image, int x_min, int z_min, int x_tot, int z_tot);

struct region {
    int x,z;
    bool done;
};

struct region_size {
    int x_max,x_min,z_max,z_min;
    int x_tot,z_tot;
};

bool isDir(string dir)
{
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);
    if (S_ISDIR(fileInfo.st_mode)) {
        return true;
    } else {
        return false;
    }
}

//Argument should contain the trailing slash ( eg. /etc/httpd/, NOT /etc/httpd ).
int read_tiles2(string baseDir, std::map<long long int, region> &regions_mapped, bool recursive)
{
    baseDir=baseDir + "/";
    //printf("\nReading %sr.*.*.png\n",baseDir.c_str());
    struct region one_region;

    DIR *dp;
    struct dirent *dirp;
    int count=0;
    int count2=1;
    if ((dp = opendir(baseDir.c_str())) == NULL) {
        cout << "[ERROR: " << errno << " ] Couldn't open " << baseDir << "." << endl;
        return -1;
    } else {
        while ((dirp = readdir(dp)) != NULL) {
            if (dirp->d_name != string(".") && dirp->d_name != string("..")) {
                if (isDir(baseDir + dirp->d_name) == true && recursive == true) {
//                    cout << "[DIR]\t" << baseDir << dirp->d_name << "/" << endl;
                    read_tiles2(baseDir + dirp->d_name , regions_mapped, true);
                } else {
                    if ((strstr(dirp->d_name, ".png")) != NULL) {
                        if ( sscanf(dirp->d_name,"r.%d.%d.png",&one_region.x,&one_region.z) == 2 ) {
                            if (!((++count)%count2)) {
                                printf("%s%s %d   \r",baseDir.c_str(),dirp->d_name,count);
                                if (!(count%(count2*100)))
                                    count2*=10;
                            }
                            one_region.done=false;
                            regions_mapped.insert(std::make_pair((long long)one_region.z*1000000+(long long)one_region.x, one_region));
                        } else {
                            printf("Wrong filename format: %s\n",dirp->d_name);
                        }
                    }

//                    cout << "[FILE]\t" << baseDir << dirp->d_name << endl;
                    count++;
                }
            }
        }
        closedir(dp);
    }
    if (count>0) printf("%sr.%d.%d.png SubTotal=%d Total= %d files       \n",baseDir.c_str(), one_region.x,one_region.z,count, regions_mapped.size());

    return regions_mapped.size();
}


struct stat stat_buffer;

bool file_exists(const char * filename)
{
    int exist = stat(filename,&stat_buffer);
    if (exist==0) return true;
    return false;
}


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
    make_border(m_image,x_min,z_min,x_tot,z_tot);
    fclose(out);
    fclose(wrong);
    printf("\n");

    sf::Texture m_texture;
    m_texture.loadFromImage(m_image);

    float aspect_ratio=x_tot/z_tot;

    sf::Shader m_shader;
    m_shader.loadFromFile("template.vert","template.frag");
//    m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));
//    m_shader.setUniform("iResolution", sf::Vector2f(m_texture.getSize()));

    float xt,yt,a;
    a=atan2(-22327,71921);

    xt=m_texture.getSize().x/2.0;
    yt=m_texture.getSize().y/2.0;
    float x1=xt*cos(a)-yt*sin(a);
    float y1=xt*sin(a)+yt*cos(a);
    yt=-yt;
    float x2=xt*cos(a)-yt*sin(a);
    float y2=xt*sin(a)+yt*cos(a);

    int m_size_x, m_size_y;

    if (abs(x1)>abs(x2)) m_size_x=int(abs(x1)*2.0+2.0);
    else m_size_x=int(abs(x2)*2.0+2.0);

    if (abs(y1)>abs(y2)) m_size_y=int(abs(y1)*2.0+2.0);
    else m_size_y=int(abs(y2)*2.0+2.0);

//    m_shader.setUniform("iResolution", sf::Vector2f(m_size_x,m_size_y));
    m_shader.setUniform("iResolution", sf::Vector2f(m_texture.getSize()));

//    sf::RenderWindow sfmlWin(sf::VideoMode(x_tot, z_tot), "PACMAN", sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close);
    sf::RenderWindow sfmlWin(sf::VideoMode(m_size_x, m_size_y), "PACMAN", sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close);

    sf::RenderTexture m_render_texture;
//    m_render_texture.create(m_texture.getSize().x,m_texture.getSize().y);
    m_render_texture.create(m_size_x, m_size_y);

    sf::Sprite m_render_sprite=sf::Sprite(m_render_texture.getTexture());
    m_render_sprite.setOrigin(
        m_size_x/2,
        m_size_y/2
//        m_render_texture.getTexture().getSize().x/2,
//        m_render_texture.getTexture().getSize().y/2
    );
    m_render_sprite.setPosition(
        m_size_x/2,
        m_size_y/2
//        m_render_texture.getTexture().getSize().x/2,
//        m_render_texture.getTexture().getSize().y/2
    );

    sf::Sprite m_sprite=sf::Sprite(m_texture);
    m_sprite.setOrigin(
//        m_size_x/2,
//        m_size_y/2
        m_texture.getSize().x/2,
        m_texture.getSize().y/2
    );
    m_sprite.setPosition(
        m_size_x/2,
        m_size_y/2
//        m_texture.getSize().x/2,
//        m_texture.getSize().y/2
    );

    m_shader.setUniform("iResolution", sf::Vector2f(m_texture.getSize()));
    m_sprite.setRotation((180.0/M_PI)*a);
    m_render_texture.draw(m_sprite);
    m_render_texture.display();
    m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));
    m_render_texture.getTexture().copyToImage().saveToFile("output.png");

    sfmlWin.clear();
//    sfmlWin.draw(m_sprite);
    sfmlWin.draw(m_render_sprite,&m_shader);
    sfmlWin.display();
    sfmlWin.setVerticalSyncEnabled(true);
    sf::sleep(sf::seconds(2.0));

    while (sfmlWin.isOpen()) {
        //sf::sleep(sf::seconds(0.1));
        sf::Event e;
        if (sfmlWin.pollEvent(e)) {
            static float m_angle=(180.0/M_PI)*a;


            switch (e.type) {
            case sf::Event::EventType::KeyPressed:
                switch(e.key.code) {
                case sf::Keyboard::Escape: {
                        sfmlWin.close();
                        break;
                    }
                case sf::Keyboard::I: {
                        sf::Texture m_save;
                        m_save.create(sfmlWin.getSize().x,sfmlWin.getSize().y);
                        m_save.update(sfmlWin,0,0);

                        m_save.copyToImage().saveToFile("output_window.png");
                    }
                    break;
                case sf::Keyboard::Space:
                    m_angle=0.0;
                    m_sprite.setRotation(m_angle);
                    m_render_texture.clear();
                    m_shader.setUniform("iResolution", sf::Vector2f(m_texture.getSize()));
                    m_render_texture.draw(m_sprite);
                    m_render_texture.display();
                    m_shader.setUniform("iResolution", sf::Vector2f(sfmlWin.getSize()));
                    break;
                case sf::Keyboard::PageUp:
                case sf::Keyboard::PageDown:
                    if (e.key.code==sf::Keyboard::PageUp)
                        if (e.key.control)
                            m_angle=m_angle+0.1;
                        else
                            m_angle=m_angle+1.0;
                    else
                        if (e.key.control)
                            m_angle=m_angle-0.1;
                        else
                            m_angle=m_angle-1.0;
                    m_sprite.setRotation(m_angle);
                    m_render_texture.clear();
                    m_shader.setUniform("iResolution", sf::Vector2f(m_texture.getSize()));
                    m_render_texture.draw(m_sprite);
                    m_render_texture.display();
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
        //sfmlWin.draw(m_sprite2,&m_shader);
//        sfmlWin.draw(m_sprite2,&m_shader);
        sfmlWin.clear();
        sfmlWin.draw(m_render_sprite,&m_shader);
        sfmlWin.display();
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

    sf::RenderWindow Window_OUT(sf::VideoMode(m_image.getSize().x,m_image.getSize().y), "PACMAN2", sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close);

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
                    Window_OUT.draw(m_sprite2);
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
                    Window_OUT.display();

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

int read_tiles(char* directory, std::map<long long int, region> &regions_mapped, struct region_size &region_size) {

    //std::map<long long int, region> regions_mapped;
    //struct region_size region_size;

    DIR* m_dir;
    struct dirent *m_dirent;

    printf("\nReading %s/r.*.*.png\n",directory);
    m_dir = opendir(directory);
    struct region one_region;

    int x_max=-std::numeric_limits<int>::max();
    int x_min=std::numeric_limits<int>::max();
    int z_max=-std::numeric_limits<int>::max();
    int z_min=std::numeric_limits<int>::max();

    int count=0;
    int count2=1;
    char* name;
    m_dirent = readdir(m_dir);
    while (m_dirent != NULL) {
        name=m_dirent->d_name;
        if ((strstr(name, ".png")) != NULL) {
            if ( sscanf(name,"r.%d.%d.png",&one_region.x,&one_region.z) == 2 ) {
                if (!((++count)%count2)) {
                    printf("%s %d   \r",name,count);
                    if (!(count%(count2*100)))
                        count2*=10;
                }
                one_region.done=false;
                regions_mapped.insert(std::make_pair((long long)one_region.z*1000000+(long long)one_region.x, one_region));
                x_max=std::max( x_max , one_region.x);
                x_min=std::min( x_min , one_region.x);
                z_max=std::max( z_max , one_region.z);
                z_min=std::min( z_min , one_region.z);
            } else {
                printf("Wrong filename format: %s\n",name);
            }

//            std::map<long long int, region>::iterator it;
//            it=regions_mapped.begin();
//            while (it != regions_mapped.end()) {
//                printf("r.%d.%d.png\n", it->second.x, it->second.z);
//                it++;
//            }


//            it_regions_mapped=regions_mapped.find(one_region.z*1000000+one_region.x);
//            if (it_regions_mapped != regions_mapped.end()) {
//                printf("r.%d.%d.png\n", it->second.x, it->second.z);
//            } else {
//                printf("Error\n");
//            }

        }
        m_dirent = readdir(m_dir);
    }
    if (count>0) printf("r.%d.%d.png Total=%d files\n",one_region.x,one_region.z,count);

    closedir(m_dir);
    region_size.x_max=x_max;
    region_size.x_min=x_min;
    region_size.z_max=z_max;
    region_size.z_min=z_min;

    return regions_mapped.size();
}

bool TILE(  int& x2, int& z2,
            int& x_start, int& z_start,
            int &zoom_level,
            char* directory_from,
            sf::RenderWindow& Window_IN,
            sf::RenderWindow& Window_OUT
        )
{
    sf::Event e;

    int x10000; //store dir
    int z10000;

    if (x2>=0) x10000=x2/10; else x10000=(x2+1)/10-1;
    if (z2>=0) z10000=z2/10; else z10000=(z2+1)/10-1;

    char d[30];
    sprintf(d,"%d/%d", zoom_level-1, x10000);
    if (!file_exists(d)) mkdir(d);
    sprintf(d,"%d/%d/%d", zoom_level-1, x10000, z10000);
    if (!file_exists(d)) mkdir(d);

    printf("Loading from dir %s: ",directory_from);
    static bool first=true;
    static sf::Sprite sprite_in[2][2];
    static sf::Sprite sprite_out[2][2];
    static sf::Texture in[2][2];
    static sf::Texture skull;
    static sf::Texture pacman;
    static sf::RenderTexture out;
    static sf::Sprite sprout;

    int map2x2[2][2];
    if (first) {
        first=false;
        pacman.create(512,512);
        pacman.loadFromFile("PACMAN.PNG");
        skull.create(512,512);
        skull.loadFromFile("SKULL.PNG");
        out.create(512,512);
        out.clear(sf::Color(0,0,0,255));
        sprout.setTexture(out.getTexture(),true);
        out.setSmooth(true);
        for (int z_c=0; z_c<2; z_c++) {
            for (int x_c=0; x_c<2; x_c++) {
                in[x_c][z_c].create(512,512);
                in[x_c][z_c].update(skull);
                sprite_out[x_c][z_c].setTexture(in[x_c][z_c],true);
                sprite_out[x_c][z_c].setOrigin(256,256);
                sprite_out[x_c][z_c].setPosition(x_c*256+128,z_c*256+128);
                sprite_out[x_c][z_c].setTextureRect({0,0,512,512});
                sprite_out[x_c][z_c].setScale(0.5,0.5);
                in[x_c][z_c].setSmooth(true);
                sprite_in[x_c][z_c].setTexture(in[x_c][z_c],true);
                sprite_in[x_c][z_c].setPosition(0,0);
                sprite_in[x_c][z_c].setOrigin(0,0);
                sprite_in[x_c][z_c].setTextureRect({0,0,512,512});
            }
        }
    }
    char filename_from[1000];
    char filename_to[1000];
    Window_OUT.clear(sf::Color(0,0,0,255));
    out.clear(sf::Color(0,0,0,255));
    for (int z_c=0; z_c<2; z_c++) {
        for (int x_c=0; x_c<2; x_c++) {
            Window_IN.pollEvent(e);
            Window_OUT.pollEvent(e);
            sprintf(filename_from,"%s/r.%d.%d.png", directory_from, x_start+x_c, z_start+z_c);
            if (file_exists(filename_from)) {
                printf("r.%d.%d.png ", x_start+x_c, z_start+z_c);
                if (!(in[x_c][z_c].loadFromFile(filename_from))) {
                    printf("Error reading tile: %s\n",filename_from);
                    printf(filename_to,"echo Error reading tile: %s>>error.log",filename_from);
                    system(filename_to);
                    map2x2[x_c][z_c]=-1;
                    in[x_c][z_c].update(skull);
                } else {
                    map2x2[x_c][z_c]=1;
                }
            } else {
                printf("R.%d.%d.MIS ", x_start+x_c, z_start+z_c);
                map2x2[x_c][z_c]=0;
                in[x_c][z_c].update(pacman);
            }
            if (x_c+z_c!=2) printf("+ ");
            Window_IN.clear(sf::Color(0,0,0,255));
            Window_IN.draw(sprite_in[x_c][z_c]);
            Window_IN.display();
//            for (int t=0; t<100; t++) { sf::sleep(sf::seconds(0.01)); Window_IN.pollEvent(e); Window_OUT.pollEvent(e); }

            out.draw(sprite_out[x_c][z_c]);
//            for (int t=0; t<100; t++) { sf::sleep(sf::seconds(0.01)); Window_IN.pollEvent(e); Window_OUT.pollEvent(e); }
        }
    }
    out.display();
    Window_OUT.draw(sprout);
    Window_OUT.display();

    sprintf(filename_to,"%s/r.%d.%d.png",d, x2, z2);
    printf("to %s Saving...",filename_to);

/*    printf("\nMERGING FROM DIR %s: ",directory_from);

    Window_OUT.clear(sf::Color(255,255,0));
    out.clear(sf::Color(255,0,255));
    for (int z_c=0; z_c<2; z_c++) {
        for (int x_c=0; x_c<2; x_c++) {
            if (map2x2[x_c][z_c]==1) {
                printf("R.%d.%d.PNG ", x_start+x_c, z_start+z_c);
            } else if (map2x2[x_c][z_c]==0) {
                printf("R.%d.%d.MIS ", x_start+x_c, z_start+z_c);
            } else {
                printf("R.%d.%d.ERR ", x_start+x_c, z_start+z_c);
            }
            out.draw(sprite_out[x_c][z_c]);
            out.display();
            Window_OUT.draw(sprout);
            Window_OUT.display();
            if (x_c+z_c!=2) printf("+ ");
//            for (int t=0; t<100; t++) { sf::sleep(sf::seconds(0.01)); Window_IN.pollEvent(e); Window_OUT.pollEvent(e); }
        }
    }
*/
//    printf("TO %s ",filename_to);
//    out.display();
//    printf("Saving...");
    while (Window_IN.pollEvent(e));
    while (Window_OUT.pollEvent(e));
    if (out.getTexture().copyToImage().saveToFile(filename_to)) {
        printf("Ok.\n");
        while (Window_IN.pollEvent(e));
        while (Window_OUT.pollEvent(e));
        return true;
    } else {
        printf("ERROR SAVING!\n");
        printf(filename_from,"echo Error saving tile: %s>>error.log",filename_to);
        while (Window_IN.pollEvent(e));
        while (Window_OUT.pollEvent(e));
        system(filename_from);
        return false;
    }
//            for (int t=0; t<100; t++) { sf::sleep(sf::seconds(0.01)); Window_IN.pollEvent(e); Window_OUT.pollEvent(e); }
}

bool make_new_tile( char* directory,
                    std::map<long long int, region> &regions_mapped,
                    struct region* region_pointer,
                    int& zoom_level,
                    sf::RenderWindow& Window_IN, sf::RenderWindow& Window_OUT
                    )
{
    int x=region_pointer->x;
    int z=region_pointer->z;
    printf("\nWORKING ON: %s/r.%d.%d.png\n", directory, x, z);

    int x2; //next level
    int z2;

    if (x>=0) x2=x/2; else x2=(x+1)/2-1;
    if (z>=0) z2=z/2; else z2=(z+1)/2-1;

    int x100; //store dir
    int z100;
    if (x>=0) x100=x/10; else x100=(x+1)/10-1;
    if (z>=0) z100=z/10; else z100=(z+1)/10-1;

    char d[30];
    if (zoom_level==11) {
        sprintf(d,"%d/%d", zoom_level, x100);
        if (!file_exists(d)) mkdir(d);
    }
    sprintf(d,"%d/%d/%d", zoom_level, x100, z100);
    if (zoom_level==11) {
        if (!file_exists(d)) mkdir(d);
    }

    int x_start=x2*2;
    int z_start=z2*2;
    static char filename_from[1000];
    static char filename_to[1000];
    std::map<long long int, region>::iterator it;
    sf::Event e;
    for (int z_c=0; z_c<2; z_c++) {
        for (int x_c=0; x_c<2; x_c++) {
            while (Window_IN.pollEvent(e));
            while (Window_OUT.pollEvent(e));

            it = regions_mapped.find((long long)(z_start+z_c)*1000000+(long long)(x_start+x_c));
            if (it != regions_mapped.end()) {
                if (zoom_level==11) { // create first new copy
                    sprintf(filename_from,"%s/r.%d.%d.png", directory, x_start+x_c, z_start+z_c);
                    sprintf(filename_to,"%s/r.%d.%d.png", d, x_start+x_c, z_start+z_c);
                    printf("Copying %s to %s", filename_from, filename_to);
                    if (CopyFile(filename_from,filename_to,false)==0) {
                        printf(" -> Can not copy %s to %s\n", filename_from, filename_to);
                        return false;
                    }
                    printf(" -> Ok.\n", filename_from, filename_to);
                }
                it->second.done=true;
            } else {
                if (zoom_level==11)
                    printf("Missing %s/r.%d.%d.png\n", directory, x_start+x_c, z_start+z_c);
                else
                    printf("Missing %s/r.%d.%d.png\n", d, x_start+x_c, z_start+z_c);
            }
        }
    }
    return TILE(x2, z2, x_start, z_start, zoom_level, d, Window_IN, Window_OUT);
    //printf("r.%d.%d.png\n", region_pointer->x, region_pointer->z);
}

int make_tiles(int zoom_level, char* directory, std::map<long long int, region> &regions_mapped) {

    //std::map<long long int, region> regions_mapped;
    //struct region_size region_size;

    sf::RenderWindow Window_IN( sf::VideoMode(512, 512), "IN",  sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close );
    sf::RenderWindow Window_OUT(sf::VideoMode(512, 512), "OUT", sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close );

    Window_IN.setVerticalSyncEnabled(false);
    Window_OUT.setVerticalSyncEnabled(false);

//    int zoom_level=11;
    struct region* region_pointer;

    std::map<long long int, region>::iterator it;
    it=regions_mapped.begin();

    char zl[10];
    sprintf(zl,"%d",zoom_level);
    if (!file_exists(zl)) mkdir(zl);
    sprintf(zl,"%d",zoom_level-1);
    if (!file_exists(zl)) mkdir(zl);

    sf::Event e;

    while (it != regions_mapped.end()) {
        region_pointer=&it->second;
        if (region_pointer->done==false) {
            while (Window_IN.pollEvent(e));
            while (Window_OUT.pollEvent(e));
            if (!make_new_tile(directory, regions_mapped, region_pointer, zoom_level, Window_IN, Window_OUT)) {
                printf("make_new_tile(): Ërror\n");
                Window_IN.close();
                Window_OUT.close();
                return -1;
            }
        }
        //printf("r.%d.%d.png\n", region_pointer->x, region_pointer->z);
        it++;
    }


//            it_regions_mapped=regions_mapped.find(one_region.z*1000000+one_region.x);
//            if (it_regions_mapped != regions_mapped.end()) {
//                printf("r.%d.%d.png\n", it->second.x, it->second.z);
//            } else {
//                printf("Error\n");
//            }

    Window_IN.close();
    Window_OUT.close();
    return 0;
}


int main(int argc, char ** argv) {


//    for (int n=0; n<argc; n++) printf("argv[%d]=%s\n",n,argv[n]);
//    getchar();
/* Huh?
    int x;
    for (x=20; x>-126; x--) {
        int y;
        if (x>=0) y=x/10; else y=(x+1)/10-1;
        printf("%d/10 = %d\n",x,y);
    }
    for (x=20; x>-126; x--) {
        int y;
        if (x>=0) y=x/2; else y=(x+1)/2-1;
        printf("%d/10 = %d\n",x,y);
    }
    exit(0);
*/

    if (!(argc>1)) {
        printf("Usage: %s out.txt | <region_dir> [<voxels_done_dir>]\n",argv[0]);
        printf("Or: %s tiles <tile_dir>\n",argv[0]);
        return -1;
    }

    if (strcmp(argv[1],"test")==0) {
        quick_window("region.png");
        return 0;
    }

    std::map<long long int, region> regions_mapped;
    struct region_size region_size;

    if (strcmp(argv[1],"tiles")==0) {
        if (argc>1) {
            int zoom_level=0;
            sscanf(argv[2],"%d",&zoom_level);
//            if (zoom_level==0) zoom_level=11;
            printf("zoom_level=%d\n",zoom_level);
            int ret;
            if (zoom_level==0) {
                zoom_level=11;
                ret=read_tiles(argv[2], regions_mapped, region_size);
            } else {
                ret=read_tiles2(argv[2], regions_mapped, true);
            }
            if (ret>0) {
                printf("%d r.x.z.png tiles found!\n",ret);
                make_tiles(zoom_level,argv[2], regions_mapped );
                return 0;
            } else {
                printf("No r.x.z.png tiles found!\n");
                return -1;
            }
        } else {
            printf("Usage: %s tiles <tile_dir>\n",argv[0]);
            return -1;
        }
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

    std::map<long long int, region> voxels_mapped;


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

void make_border(sf::Image m_image, int x_min, int z_min, int x_tot, int z_tot) {
    int border[][2] = {
        { 6573, -10634 },
        { 6567, -10645 },
        { 6566, -10652 },
        { 6560, -10655 },
        { 6552, -10659 },
        { 6544, -10666 },
        { 6542, -10674 },
        { 6543, -10688 },
        { 6541, -10697 },
        { 6529, -10706 },
        { 6522, -10716 },
        { 6513, -10729 },
        { 6499, -10740 },
        { 6475, -10737 },
        { 6423, -10745 },
        { 6393, -10751 },
        { 6345, -10757 },
        { 6308, -10756 },
        { 6274, -10748 },
        { 6243, -10734 },
        { 6204, -10691 },
        { 6195, -10679 },
        { 6196, -10657 },
        { 6176, -10626 },
        { 6150, -10577 },
        { 6120, -10534 },
        { 6079, -10502 },
        { 6048, -10483 },
        { 6032, -10486 },
        { 6024, -10475 },
        { 6026, -10461 },
        { 6020, -10455 },
        { 6004, -10453 },
        { 5993, -10443 },
        { 5980, -10444 },
        { 5967, -10434 },
        { 5963, -10420 },
        { 5953, -10416 },
        { 5935, -10416 },
        { 5919, -10411 },
        { 5921, -10399 },
        { 5925, -10381 },
        { 5919, -10376 },
        { 5908, -10377 },
        { 5901, -10372 },
        { 5901, -10370 },
        { 5901, -10367 },
        { 5901, -10365 },
        { 5901, -10364 },
        { 5896, -10360 },
        { 5897, -10356 },
        { 5895, -10356 },
        { 5897, -10351 },
        { 5895, -10351 },
        { 5897, -10349 },
        { 5899, -10345 },
        { 5900, -10345 },
        { 5901, -10341 },
        { 5903, -10339 },
        { 5914, -10337 },
        { 5916, -10346 },
        { 5917, -10346 },
        { 5919, -10345 },
        { 5921, -10346 },
        { 5922, -10345 },
        { 5923, -10346 },
        { 5924, -10345 },
        { 5925, -10344 },
        { 5926, -10346 },
        { 5928, -10347 },
        { 5933, -10341 },
        { 5935, -10341 },
        { 5939, -10337 },
        { 5942, -10334 },
        { 5948, -10331 },
        { 5949, -10329 },
        { 5951, -10329 },
        { 5953, -10327 },
        { 5950, -10323 },
        { 5950, -10317 },
        { 5953, -10315 },
        { 5955, -10315 },
        { 5959, -10314 },
        { 5964, -10315 },
        { 5964, -10313 },
        { 5961, -10312 },
        { 5961, -10310 },
        { 5963, -10310 },
        { 5966, -10310 },
        { 5967, -10312 },
        { 5968, -10312 },
        { 5970, -10310 },
        { 5973, -10310 },
        { 5974, -10311 },
        { 5977, -10312 },
        { 5979, -10313 },
        { 5983, -10314 },
        { 5986, -10313 },
        { 5988, -10312 },
        { 5991, -10313 },
        { 6007, -10318 },
        { 6023, -10328 },
        { 6021, -10331 },
        { 6021, -10334 },
        { 6021, -10334 },
        { 6038, -10329 },
        { 6037, -10325 },
        { 6038, -10324 },
        { 6040, -10324 },
        { 6041, -10323 },
        { 6043, -10322 },
        { 6044, -10322 },
        { 6049, -10323 },
        { 6050, -10322 },
        { 6050, -10324 },
        { 6051, -10324 },
        { 6048, -10334 },
        { 6048, -10337 },
        { 6049, -10338 },
        { 6050, -10338 },
        { 6050, -10339 },
        { 6050, -10339 },
        { 6051, -10339 },
        { 6050, -10340 },
        { 6051, -10341 },
        { 6051, -10342 },
        { 6050, -10342 },
        { 6049, -10343 },
        { 6050, -10344 },
        { 6050, -10343 },
        { 6060, -10345 },
        { 6063, -10345 },
        { 6064, -10345 },
        { 6065, -10346 },
        { 6066, -10345 },
        { 6074, -10344 },
        { 6075, -10341 },
        { 6070, -10337 },
        { 6069, -10331 },
        { 6076, -10331 },
        { 6077, -10330 },
        { 6083, -10327 },
        { 6084, -10326 },
        { 6085, -10326 },
        { 6086, -10326 },
        { 6087, -10326 },
        { 6088, -10325 },
        { 6089, -10329 },
        { 6092, -10330 },
        { 6093, -10330 },
        { 6096, -10333 },
        { 6098, -10333 },
        { 6098, -10333 },
        { 6101, -10336 },
        { 6104, -10336 },
        { 6105, -10337 },
        { 6105, -10338 },
        { 6106, -10338 },
        { 6106, -10338 },
        { 6107, -10338 },
        { 6109, -10338 },
        { 6109, -10338 },
        { 6110, -10337 },
        { 6111, -10336 },
        { 6112, -10336 },
        { 6114, -10334 },
        { 6114, -10331 },
        { 6117, -10330 },
        { 6115, -10326 },
        { 6115, -10325 },
        { 6114, -10324 },
        { 6114, -10325 },
        { 6112, -10323 },
        { 6111, -10320 },
        { 6110, -10317 },
        { 6111, -10317 },
        { 6112, -10317 },
        { 6111, -10316 },
        { 6110, -10317 },
        { 6109, -10319 },
        { 6106, -10321 },
        { 6105, -10321 },
        { 6102, -10322 },
        { 6102, -10320 },
        { 6102, -10319 },
        { 6102, -10318 },
        { 6104, -10316 },
        { 6105, -10316 },
        { 6112, -10315 },
        { 6113, -10314 },
        { 6113, -10314 },
        { 6114, -10314 },
        { 6114, -10313 },
        { 6115, -10313 },
        { 6115, -10313 },
        { 6116, -10313 },
        { 6117, -10313 },
        { 6117, -10313 },
        { 6118, -10313 },
        { 6119, -10312 },
        { 6120, -10312 },
        { 6120, -10311 },
        { 6121, -10312 },
        { 6120, -10311 },
        { 6121, -10310 },
        { 6121, -10309 },
        { 6121, -10309 },
        { 6121, -10308 },
        { 6122, -10308 },
        { 6122, -10307 },
        { 6121, -10307 },
        { 6122, -10307 },
        { 6122, -10307 },
        { 6122, -10308 },
        { 6123, -10308 },
        { 6123, -10307 },
        { 6128, -10310 },
        { 6137, -10315 },
        { 6139, -10319 },
        { 6144, -10323 },
        { 6145, -10319 },
        { 6150, -10317 },
        { 6150, -10307 },
        { 6143, -10300 },
        { 6148, -10287 },
        { 6146, -10279 },
        { 6149, -10277 },
        { 6156, -10278 },
        { 6160, -10272 },
        { 6156, -10264 },
        { 6157, -10263 },
        { 6160, -10263 },
        { 6165, -10260 },
        { 6171, -10259 },
        { 6173, -10261 },
        { 6182, -10255 },
        { 6187, -10258 },
        { 6191, -10258 },
        { 6194, -10260 },
        { 6199, -10258 },
        { 6199, -10255 },
        { 6202, -10250 },
        { 6199, -10240 },
        { 6211, -10230 },
        { 6211, -10227 },
        { 6213, -10226 },
        { 6217, -10224 },
        { 6221, -10223 },
        { 6224, -10224 },
        { 6227, -10220 },
        { 6227, -10219 },
        { 6226, -10218 },
        { 6227, -10217 },
        { 6225, -10214 },
        { 6231, -10215 },
        { 6231, -10214 },
        { 6233, -10215 },
        { 6236, -10209 },
        { 6233, -10207 },
        { 6231, -10207 },
        { 6228, -10205 },
        { 6227, -10203 },
        { 6229, -10202 },
        { 6230, -10200 },
        { 6228, -10198 },
        { 6226, -10200 },
        { 6224, -10200 },
        { 6225, -10196 },
        { 6223, -10195 },
        { 6222, -10192 },
        { 6219, -10194 },
        { 6217, -10191 },
        { 6215, -10188 },
        { 6216, -10185 },
        { 6215, -10184 },
        { 6214, -10181 },
        { 6209, -10177 },
        { 6204, -10174 },
        { 6205, -10172 },
        { 6208, -10172 },
        { 6209, -10170 },
        { 6203, -10165 },
        { 6202, -10162 },
        { 6200, -10162 },
        { 6198, -10163 },
        { 6197, -10161 },
        { 6195, -10159 },
        { 6193, -10157 },
        { 6188, -10157 },
        { 6187, -10156 },
        { 6185, -10151 },
        { 6185, -10145 },
        { 6187, -10143 },
        { 6190, -10140 },
        { 6189, -10138 },
        { 6189, -10134 },
        { 6184, -10130 },
        { 6186, -10128 },
        { 6190, -10129 },
        { 6192, -10127 },
        { 6199, -10131 },
        { 6200, -10127 },
        { 6201, -10127 },
        { 6202, -10123 },
        { 6204, -10123 },
        { 6207, -10124 },
        { 6208, -10121 },
        { 6214, -10123 },
        { 6215, -10119 },
        { 6216, -10118 },
        { 6217, -10117 },
        { 6225, -10117 },
        { 6226, -10115 },
        { 6232, -10114 },
        { 6235, -10117 },
        { 6229, -10125 },
        { 6231, -10127 },
        { 6233, -10125 },
        { 6238, -10128 },
        { 6238, -10134 },
        { 6243, -10135 },
        { 6247, -10132 },
        { 6251, -10138 },
        { 6250, -10141 },
        { 6254, -10148 },
        { 6245, -10154 },
        { 6245, -10160 },
        { 6249, -10165 },
        { 6242, -10166 },
        { 6239, -10169 },
        { 6233, -10168 },
        { 6230, -10170 },
        { 6233, -10175 },
        { 6231, -10179 },
        { 6232, -10183 },
        { 6231, -10187 },
        { 6235, -10187 },
        { 6239, -10188 },
        { 6240, -10181 },
        { 6243, -10180 },
        { 6244, -10181 },
        { 6246, -10183 },
        { 6249, -10187 },
        { 6253, -10190 },
        { 6257, -10190 },
        { 6262, -10193 },
        { 6265, -10194 },
        { 6267, -10195 },
        { 6269, -10196 },
        { 6274, -10197 },
        { 6280, -10197 },
        { 6282, -10198 },
        { 6279, -10203 },
        { 6285, -10204 },
        { 6283, -10206 },
        { 6276, -10206 },
        { 6274, -10204 },
        { 6271, -10205 },
        { 6270, -10208 },
        { 6272, -10217 },
        { 6274, -10216 },
        { 6274, -10222 },
        { 6278, -10223 },
        { 6285, -10228 },
        { 6290, -10233 },
        { 6294, -10238 },
        { 6297, -10237 },
        { 6297, -10239 },
        { 6304, -10242 },
        { 6304, -10248 },
        { 6307, -10250 },
        { 6304, -10252 },
        { 6308, -10260 },
        { 6309, -10262 },
        { 6312, -10268 },
        { 6311, -10272 },
        { 6312, -10277 },
        { 6310, -10282 },
        { 6309, -10291 },
        { 6302, -10304 },
        { 6309, -10314 },
        { 6299, -10321 },
        { 6300, -10330 },
        { 6303, -10330 },
        { 6296, -10338 },
        { 6292, -10339 },
        { 6292, -10342 },
        { 6299, -10344 },
        { 6298, -10346 },
        { 6300, -10348 },
        { 6298, -10349 },
        { 6299, -10352 },
        { 6296, -10357 },
        { 6298, -10357 },
        { 6297, -10359 },
        { 6300, -10361 },
        { 6305, -10359 },
        { 6310, -10360 },
        { 6316, -10364 },
        { 6320, -10358 },
        { 6324, -10357 },
        { 6329, -10354 },
        { 6330, -10358 },
        { 6328, -10360 },
        { 6328, -10365 },
        { 6324, -10368 },
        { 6327, -10369 },
        { 6332, -10369 },
        { 6336, -10364 },
        { 6334, -10363 },
        { 6338, -10358 },
        { 6340, -10358 },
        { 6344, -10355 },
        { 6346, -10356 },
        { 6349, -10355 },
        { 6349, -10350 },
        { 6356, -10348 },
        { 6357, -10343 },
        { 6362, -10341 },
        { 6364, -10346 },
        { 6363, -10350 },
        { 6373, -10344 },
        { 6407, -10349 },
        { 6412, -10342 },
        { 6423, -10349 },
        { 6431, -10355 },
        { 6431, -10363 },
        { 6416, -10377 },
        { 6417, -10383 },
        { 6424, -10382 },
        { 6431, -10390 },
        { 6445, -10388 },
        { 6455, -10397 },
        { 6463, -10398 },
        { 6471, -10405 },
        { 6482, -10406 },
        { 6480, -10417 },
        { 6490, -10429 },
        { 6494, -10440 },
        { 6490, -10446 },
        { 6488, -10463 },
        { 6481, -10456 },
        { 6470, -10465 },
        { 6449, -10478 },
        { 6451, -10494 },
        { 6462, -10493 },
        { 6459, -10501 },
        { 6461, -10512 },
        { 6472, -10512 },
        { 6496, -10505 },
        { 6508, -10498 },
        { 6527, -10546 },
        { 6537, -10554 },
        { 6555, -10576 },
        { 6561, -10600 },
        { 6560, -10608 },
        { 6569, -10618 },
        { 6573, -10634 },
        {    0,      0 }
     };

    sf::VertexArray lines(sf::LineStrip);
    int x,z,n=0;
    sf::Vertex m_vertex;

    do {
        x=border[n][0];
        z=border[n][1];
        if (x!=0 || z!=0) {
            m_vertex.position=sf::Vector2f(x-x_min, z-z_min);
            m_vertex.color=sf::Color(0,0,0,255);
            lines.append(m_vertex);
        }
        n++;
    } while (x!=0 || z!=0);

    sf::Texture m_texture;
    m_texture.loadFromImage(m_image);

    sf::Sprite m_sprite;
    m_sprite.setTexture(m_texture,true);

    sf::RenderTexture m_render;
    m_render.create(x_tot,z_tot);
    m_render.clear(sf::Color(0,0,0,0));
    m_render.draw(m_sprite);
    m_render.draw(lines);
    m_render.display();
    m_texture.update(m_render.getTexture());
    m_image=m_texture.copyToImage();

    m_texture.copyToImage().saveToFile("borders.png");

    x=x_tot/2;
    z=z_tot/2;
    m_image.setPixel(x,z,sf::Color(255,127,0,255));
    m_image.setPixel(270,440,sf::Color(255,127,0,255));
    bool not_done=true;
    sf::Color pixel;
    while (not_done) {
        not_done=false;
        n++;
        if (n>100) {
            printf(".");
            n=0;
        }
        for (z=0; z<z_tot; z++) {
            for (x=0; x<x_tot; x++) {
                pixel=m_image.getPixel(x,z);
                if (pixel.r==255 && pixel.g==127 && pixel.b==0) {
                    if (x>0) {
                        pixel=m_image.getPixel(x-1,z  );
                        if ( (pixel.r!=0 || pixel.g!=0 || pixel.b!=0) && !(pixel.r==255 && pixel.g==127 && pixel.b==0) ) {
                            m_image.setPixel(x-1,z  ,sf::Color(255,127,0,255));
                            not_done=true;
                        }
                    }
                    if (x<x_tot-1) {
                        pixel=m_image.getPixel(x+1,z  );
                        if ( (pixel.r!=0 || pixel.g!=0 || pixel.b!=0) && !(pixel.r==255 && pixel.g==127 && pixel.b==0) ) {
                            m_image.setPixel(x+1,z  ,sf::Color(255,127,0,255));
                            not_done=true;
                        }
                    }
                    if (z>0) {
                        pixel=m_image.getPixel(x  ,z-1);
                        if ( (pixel.r!=0 || pixel.g!=0 || pixel.b!=0) && !(pixel.r==255 && pixel.g==127 && pixel.b==0) ) {
                            m_image.setPixel(x  ,z-1,sf::Color(255,127,0,255));
                            not_done=true;
                        }
                    }
                    if (z<z_tot-1) {
                        pixel=m_image.getPixel(x  ,z+1);
                        if ( (pixel.r!=0 || pixel.g!=0 || pixel.b!=0) && !(pixel.r==255 && pixel.g==127 && pixel.b==0) ) {
                            m_image.setPixel(x  ,z+1,sf::Color(255,127,0,255));
                            not_done=true;
                        }
                    }
                }
            }
        }
    }
    printf("\n");
    m_image.saveToFile("borders_fill.png");
    FILE* f=fopen("Holland.mca","w");
    for (z=0; z<z_tot; z++) {
        for (x=0; x<x_tot; x++) {
            pixel=m_image.getPixel(x,z);
            if ( (pixel.r==255 && pixel.g==127 && pixel.b==0) ||  (pixel.r==0 && pixel.g==0 && pixel.b==0) ) {
                fprintf(f,"r.%d.%d.mca\n",x+x_min,z+z_min);
            }
        }
    }
    fclose(f);
}
