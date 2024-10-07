#version 430 core

struct Material{
    vec3 emmisionColor;
    vec3 emmisionStrength;
    vec3 diffuseColor;
    vec3 smoothness;
};

struct Sphere{
    Material material;
    vec3 position;
    vec3 radius;
};

struct Camera{
    vec3 position;
    vec3 direction;
    vec3 fov;
};

struct Ray{
    vec3 origin;
    vec3 direction;
};

struct HitInfo{
    vec3 hitPoint;
    vec3 normal;
    bool didHit;
    float dst;
    Material material;
};

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;
layout(rgba32f, binding = 6) uniform image2D oldScreen;
layout(rgba32f, binding = 7) uniform image2D averageScreen;

layout(std430, binding = 1) buffer CameraData {
    Camera camera;
};
layout(std430, binding = 2) buffer CircleData {
    Sphere spheres[];
};
layout(std430, binding = 3) buffer SphereLength {
    uint NumSpheres;
};
layout(std430, binding = 4) buffer Frames {
    uint Frame;
};

uniform mat4 viewProj;

highp float rand(inout vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt1 = dot(co.xy ,vec2(a,b));
    highp float dt2 = dot(co.xy + vec2(0.11324234,-0.21312664),vec2(a,b));
    highp float sn1= mod(dt1,3.14);
    highp float sn2= mod(dt2,3.14);

    co = vec2(fract(sin(sn1) * c), -fract(sin(sn2) * c));
    return co.x;
}

float randNorm(inout vec2 co){
    float theta = 3.14 * 2 * rand(co);
    float rho = sqrt(-2 * log(rand(co)));
    return rho * cos(theta);
}

HitInfo RaySphere(Ray ray, vec3 sphereCenter, float sphereRadius, Material material){
    HitInfo hitInfo;
    hitInfo.didHit = false;

    vec3 o_c = ray.origin - sphereCenter;
    float b = dot(ray.direction, o_c);
    float c = dot(o_c, o_c) - sphereRadius * sphereRadius;
    float intersectionState = b * b - c;
    
    if (intersectionState >= 0.0)
    {
        float dst = (-b - sqrt(intersectionState));
        vec3 intersection = ray.origin + ray.direction * dst; // fix the intersection calculation
        if(dst >= 0){
            hitInfo.didHit = true;
            hitInfo.dst = dst;
            hitInfo.hitPoint = intersection;
            hitInfo.normal = normalize(hitInfo.hitPoint - sphereCenter);
            hitInfo.material = material;
        }
    }

    return hitInfo;
}

HitInfo RayAllSpheres(Ray ray){
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.hitPoint = vec3(0.0);
    closestHit.normal = vec3(0.0);
    closestHit.dst = 1.0 / 0.0;

    for(int i = 0; i < NumSpheres; i++){
        Sphere sphere = spheres[i];
        HitInfo hitInfo = RaySphere(ray, sphere.position, sphere.radius.x, sphere.material);

        if(hitInfo.didHit && hitInfo.dst < closestHit.dst){
            closestHit = hitInfo;
        }
    }
    return closestHit;
}

vec3 RandomDirection(inout vec2 state){
    return normalize(vec3(randNorm(state), randNorm(state), randNorm(state)));
}

vec3 GetAmbientLight(Ray ray){
    float gradient = pow(smoothstep(0.0f,0.4f,float(ray.direction.y)), 0.35f);
    vec3 gradientC = mix(vec3(0,0.2,0.7), vec3(0,0.1,0.2), gradient);

    float groundToSkyT = smoothstep(-0.01f,0.0f,float(ray.direction.y));

    return mix(vec3(0.1,0.1,0.1), gradientC, groundToSkyT);
}

vec3 RandomDirectionHemsiphere(vec3 normal, inout vec2 state){
    vec3 dir = RandomDirection(state);
    return dir * sign(dot(dir,normal));
}

const int NumberOfBounces = 10;
vec3 FullTrace(Ray ray, inout vec2 state){
    vec3 rayColor = vec3(1,1,1);
    vec3 rayLight = vec3(0,0,0);

    for(int i = 0; i < NumberOfBounces; i++){
        HitInfo hitInfo = RayAllSpheres(ray);
        if(hitInfo.didHit){
            ray.origin = hitInfo.hitPoint;

            vec3 diffuseDirection = normalize(hitInfo.normal + RandomDirection(state));
            vec3 specularDirection = reflect(ray.direction, hitInfo.normal);
            ray.direction = mix(diffuseDirection, specularDirection, hitInfo.material.smoothness.x);

            vec3 emmision = hitInfo.material.emmisionColor * hitInfo.material.emmisionStrength.x;
            float lightStrength = dot(hitInfo.normal, ray.direction);
            rayLight += emmision * rayColor;
            rayColor *= hitInfo.material.diffuseColor * lightStrength;
        } else{
            rayLight += rayColor * clamp(GetAmbientLight(ray),0,1);
            break;
        }
    }

    return rayLight;
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(screen);

    // Convert pixel coordinates to normalized device coordinates (-1.0, 1.0)
    float u = (float(pixel_coords.x) / float(dims.x)) * 2.0 - 1.0;
    float v = (float(pixel_coords.y) / float(dims.y)) * 2.0 - 1.0;

    // Aspect ratio correction
    float aspectRatio = float(dims.x) / float(dims.y);
    u *= aspectRatio;

    // Calculate the direction of the ray from the camera through the screen pixel
    float fovTan = tan(radians(camera.fov.x) * 0.5);
    vec3 forward = normalize(camera.direction);
    vec3 right = normalize(cross(forward, vec3(0,1,0)));
    vec3 up = cross(right, forward);

    vec3 rayDir = normalize(forward + u * fovTan * right + v * fovTan * up);

    // Create the ray
    Ray ray;
    ray.origin = camera.position;
    ray.direction = rayDir;

    // Perform path tracing
    vec2 state = vec2(u + Frame * 0.0201489f, v + Frame * 0.0101789f);
    float numOfRays = 40;
    vec3 color = vec3(0,0,0);
    for(int i = 0; i < numOfRays; i++){
     color += FullTrace(ray, state);
    }
    color /= numOfRays; 

    // Output to image
    vec3 pixel = clamp(color, 0, 1);

    memoryBarrierImage();
    vec3 old = imageLoad(oldScreen, pixel_coords).rgb;

    float weight = 1.0 / (Frame + 1.0f);
    vec3 average = old * (1.0f - weight) + pixel * weight;

    memoryBarrierImage();
    imageStore(screen, pixel_coords, vec4(average,1));
    //imageStore(oldScreen, pixel_coords, pixel);
}