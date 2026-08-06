#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define M_PI 3.14159265358979323846f
#define MAXPARTICULAS 4000
// Numero aleatorio entre 0 e 1
#define randomico() ((float)rand() / RAND_MAX)


typedef struct _pto3f {
    float x, y, z;
} tipPto3f;

typedef struct _vert {
    int n;
    tipPto3f* vets;
} tipVertice;

typedef struct _face {
    int n;
    int* indVertice;
} tipFace;

typedef struct _objeto {
    int n;
    tipFace* face;
    tipVertice* vertice;
    int cor;
    int material;
} tipObjeto;

typedef struct _objetos {
    int m;
    tipObjeto* vobjs;
} tipObjetos;


typedef struct {
    tipPto3f posicao;
    tipPto3f velocidade;
    tipPto3f aceleracao;
    float cor[3];
    float tempoVida;       // vida restante
    float vidaInicial;     // vida inicial 
    float transparencia;
} Particula;

// Array global de particulas
Particula g_Particulas[MAXPARTICULAS];


// Variáveis globais
float g_Cores[][3] = {
    {1.0f, 0.0f, 0.0f},   // 0: Vermelho
    {0.0f, 1.0f, 0.0f},   // 1: Verde
    {0.0f, 0.0f, 1.0f},   // 2: Azul
    {1.0f, 1.0f, 0.0f},   // 3: Amarelo
    {1.0f, 0.0f, 1.0f},   // 4: Magenta
    {0.0f, 1.0f, 1.0f},   // 5: Ciano
    {1.0f, 0.5f, 0.0f},   // 6: Laranja
    {0.8f, 0.8f, 0.8f}    // 7: Cinza Claro
};

typedef struct {
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess;
} Material;

Material g_Materiais[] = {
    { {0.1f, 0.1f, 0.1f, 1.0f}, {0.7f, 0.7f, 0.7f, 1.0f}, {0.9f, 0.9f, 0.9f, 1.0f}, 32.0f },
    { {0.2f, 0.2f, 0.2f, 1.0f}, {0.8f, 0.8f, 0.8f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 80.0f },
    { {0.05f, 0.05f, 0.05f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}, {0.1f, 0.1f, 0.1f, 1.0f}, 10.0f },
    { {0.2, 0.2, 0.2, 0.5}, {0.5, 0.5f, 0.5f, 0.5f}, {1.0, 1.0, 1.0, 0.5}, 120.0}
};


tipObjetos g_Objetos;
int g_IndiceObjetoAtual = 0;
float g_rotX = 20.0f, g_rotY = 30.0f;
float g_escala = 1.0f;

// Variaveis globais
int indFaceEmissora = 0; // Face de onde as particulas saem
int tipoSim = 0;           // 0 = Fogo, 1 = Fumaca, 2 = Agua
int g_AnimacaoAtiva = 0;     // Controla se a animacao esta rodando
float raioEmissao = 0.5f; // Raio inicial do circulo emissor
float alturaNuvem = 2.0f;   // Altura da nuvem acima da face para chuva

//vento global
float vento[3] = { 0.0f, 0.0f, 0.0f };

// Tempo adaptativo
int ultimoMs = 0;


void calcularNormal(tipFace* face, tipVertice* vertices, tipPto3f* normal) {
    tipPto3f v0 = vertices->vets[face->indVertice[0]];
    tipPto3f v1 = vertices->vets[face->indVertice[1]];
    tipPto3f v2 = vertices->vets[face->indVertice[2]];
    tipPto3f vecA = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
    tipPto3f vecB = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

    normal->x = vecA.y * vecB.z - vecA.z * vecB.y;
    normal->y = vecA.z * vecB.x - vecA.x * vecB.z;
    normal->z = vecA.x * vecB.y - vecA.y * vecB.x;

    float comp = sqrtf(normal->x * normal->x + normal->y * normal->y + normal->z * normal->z);
    if (comp != 0.0f) {
        normal->x /= comp;
        normal->y /= comp;
        normal->z /= comp;
    }
}

void aplicarMaterialECor(int idMaterial, int idCor) {
    Material matBase = g_Materiais[idMaterial];
    float* cor = g_Cores[idCor];
    float mat_ambient[4], mat_diffuse[4], mat_specular[4];

    mat_ambient[0] = matBase.ambient[0] * cor[0];
    mat_ambient[1] = matBase.ambient[1] * cor[1];
    mat_ambient[2] = matBase.ambient[2] * cor[2];
    mat_ambient[3] = matBase.ambient[3];

    mat_diffuse[0] = matBase.diffuse[0] * cor[0];
    mat_diffuse[1] = matBase.diffuse[1] * cor[1];
    mat_diffuse[2] = matBase.diffuse[2] * cor[2];
    mat_diffuse[3] = matBase.diffuse[3];

    if (idMaterial == 1) {
        mat_specular[0] = matBase.specular[0] * cor[0];
        mat_specular[1] = matBase.specular[1] * cor[1];
        mat_specular[2] = matBase.specular[2] * cor[2];
    }
    else {
        mat_specular[0] = matBase.specular[0];
        mat_specular[1] = matBase.specular[1];
        mat_specular[2] = matBase.specular[2];
    }
    mat_specular[3] = matBase.specular[3];

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, matBase.shininess);
}

int carregarObjetoDeArquivo(const char* nomeArquivo, tipObjeto* obj) {
    FILE* arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return 0;
    }

    obj->vertice = (tipVertice*)malloc(sizeof(tipVertice));
    char buffer[100];
    fscanf(arquivo, "%s %d", buffer, &obj->material);
    fscanf(arquivo, "%s %d", buffer, &obj->cor);
    fscanf(arquivo, "%s %d", buffer, &obj->vertice->n);
    obj->vertice->vets = (tipPto3f*)malloc(obj->vertice->n * sizeof(tipPto3f));
    for (int i = 0; i < obj->vertice->n; i++) {
        fscanf(arquivo, "%f %f %f",
            &obj->vertice->vets[i].x,
            &obj->vertice->vets[i].y,
            &obj->vertice->vets[i].z);
    }
    fscanf(arquivo, "%s %d", buffer, &obj->n);
    obj->face = (tipFace*)malloc(obj->n * sizeof(tipFace));
    for (int i = 0; i < obj->n; i++) {
        fscanf(arquivo, "%d", &obj->face[i].n);
        obj->face[i].indVertice = (int*)malloc(obj->face[i].n * sizeof(int));
        for (int j = 0; j < obj->face[i].n; j++) {
            fscanf(arquivo, "%d", &obj->face[i].indVertice[j]);
        }
    }
    fclose(arquivo);
    printf("Objeto '%s' carregado com sucesso!\n", nomeArquivo);
    return 1;
}

void criarObjetos() {
    const char* arquivos[] = { "cubo.txt", "piramide.txt", "prisma.txt", "octaedro.txt" };
    int numArquivos = sizeof(arquivos) / sizeof(arquivos[0]);
    g_Objetos.m = numArquivos;
    g_Objetos.vobjs = (tipObjeto*)malloc(g_Objetos.m * sizeof(tipObjeto));
    if (!g_Objetos.vobjs) {
        exit(1);
    }
    for (int i = 0; i < numArquivos; i++) {
        if (!carregarObjetoDeArquivo(arquivos[i], &g_Objetos.vobjs[i])) {
            printf("Falha ao carregar %s.\n", arquivos[i]);
        }
    }
}

// encontra um indice de particula livre (vida pequena)
int encontrarParticulaLivre() {
    for (int i = 0; i < MAXPARTICULAS; i++) {
        if (g_Particulas[i].tempoVida < 0.01f) return i;
    }
    return -1;
}

// Inicializa uma unica particula (nova ou re-nascida)
void conceberParticula(int i) {
    // Encontrar a face emissora
    tipObjeto* obj = &g_Objetos.vobjs[g_IndiceObjetoAtual];

    // Garantia de seguranca
    if (indFaceEmissora >= obj->n) indFaceEmissora = 0;
    if (obj->n == 0) return; // Objeto nao foi carregado

    tipFace* face = &obj->face[indFaceEmissora];
    if (face->n < 3) return; // Face invalida

    // Calcular a centroide da face
    tipPto3f centroFace = { 0.0f, 0.0f, 0.0f };
    for (int j = 0; j < face->n; j++) {
        tipPto3f* vert = &obj->vertice->vets[face->indVertice[j]];
        centroFace.x += vert->x;
        centroFace.y += vert->y;
        centroFace.z += vert->z;
    }
    centroFace.x /= face->n;
    centroFace.y /= face->n;
    centroFace.z /= face->n;

    // Calcular a normal da face
    tipPto3f normal;
    calcularNormal(face, obj->vertice, &normal);

    // Criar uma base ortogonal (U, V) no plano da face
    tipPto3f U, V;
    tipPto3f vetQualquer = { 0.0f, 1.0f, 0.0f }; // Um vetor qualquer não paralelo a normal da face

    float dot = normal.x * vetQualquer.x + normal.y * vetQualquer.y + normal.z * vetQualquer.z;
    if (fabsf(dot) > 0.99f) {
        vetQualquer.x = 1.0f; vetQualquer.y = 0.0f; vetQualquer.z = 0.0f; // Usa o eixo X
    }

    U.x = vetQualquer.y * normal.z - vetQualquer.z * normal.y;
    U.y = vetQualquer.z * normal.x - vetQualquer.x * normal.z;
    U.z = vetQualquer.x * normal.y - vetQualquer.y * normal.x;

    float magU = sqrtf(U.x * U.x + U.y * U.y + U.z * U.z);
    if (magU > 0.0001f) { U.x /= magU; U.y /= magU; U.z /= magU; }

    V.x = normal.y * U.z - normal.z * U.y;
    V.y = normal.z * U.x - normal.x * U.z;
    V.z = normal.x * U.y - normal.y * U.x;

    // Gerar um ponto aleatorio dentro de um circulo neste plano
    float r = raioEmissao * sqrtf(randomico());
    float theta = 2.0f * M_PI * randomico();
    float cosT = cosf(theta);
    float sinT = sinf(theta);

    // Emitir de uma região acima da face
    if (tipoSim == 2) { // Chuva
        // Posição base na nuvem (acima da face)
        tipPto3f centroNuvem;
        centroNuvem.x = centroFace.x + normal.x * alturaNuvem;
        centroNuvem.y = centroFace.y + normal.y * alturaNuvem;
        centroNuvem.z = centroFace.z + normal.z * alturaNuvem;

        // Ponto aleatório na região da nuvem
        g_Particulas[i].posicao.x = centroNuvem.x + r * (cosT * U.x + sinT * V.x);
        g_Particulas[i].posicao.y = centroNuvem.y + r * (cosT * U.y + sinT * V.y);
        g_Particulas[i].posicao.z = centroNuvem.z + r * (cosT * U.z + sinT * V.z);

        // Velocidade: direção para a face (oposta à normal) + perturbações
        float velBase = 0.02f;
        float spread = 0.01f;

        g_Particulas[i].velocidade.x = -normal.x * velBase + (randomico() - 0.5f) * spread * 0.6f + vento[0] * 0.02f;
        g_Particulas[i].velocidade.y = -normal.y * velBase + (randomico() - 0.5f) * spread * 0.6f + vento[1] * 0.02f;
        g_Particulas[i].velocidade.z = -normal.z * velBase + (randomico() - 0.5f) * spread * 0.6f + vento[2] * 0.02f;

        // Gravidade age ao longo da normal (em direção à face)
        float magnitudeGravidade = 0.0035f;
        g_Particulas[i].aceleracao.x = -normal.x * magnitudeGravidade + vento[0] * 0.001f;
        g_Particulas[i].aceleracao.y = -normal.y * magnitudeGravidade + vento[1] * 0.001f;
        g_Particulas[i].aceleracao.z = -normal.z * magnitudeGravidade + vento[2] * 0.001f;

        g_Particulas[i].cor[0] = 0.05f;
        g_Particulas[i].cor[1] = 0.25f;
        g_Particulas[i].cor[2] = 0.6f + 0.4f * randomico();

        g_Particulas[i].tempoVida = 1.5f + 1.0f * randomico();
    }
    else {
        // Para fogo e fumaça, emitir da face
        g_Particulas[i].posicao.x = centroFace.x + r * (cosT * U.x + sinT * V.x);
        g_Particulas[i].posicao.y = centroFace.y + r * (cosT * U.y + sinT * V.y);
        g_Particulas[i].posicao.z = centroFace.z + r * (cosT * U.z + sinT * V.z);

        // Definir velocidade, aceleracao, cor e vida
        float velBase = 0.02f;
        float spread = 0.01f;

        if (tipoSim == 0) { // Fogo
            g_Particulas[i].velocidade.x = normal.x * velBase + (randomico() - 0.5f) * spread;
            g_Particulas[i].velocidade.y = normal.y * velBase + (randomico() - 0.5f) * spread + 0.01f * randomico();
            g_Particulas[i].velocidade.z = normal.z * velBase + (randomico() - 0.5f) * spread;

            g_Particulas[i].aceleracao.x = vento[0] * 0.001f;
            g_Particulas[i].aceleracao.y = vento[1] * 0.001f;
            g_Particulas[i].aceleracao.z = vento[2] * 0.001f;

            g_Particulas[i].cor[0] = 1.0f;
            g_Particulas[i].cor[1] = 0.8f;
            g_Particulas[i].cor[2] = 0.1f;

            g_Particulas[i].tempoVida = 0.6f + 0.6f * randomico();

        }
        else if (tipoSim == 1) { // Fumaca / Vapor
            g_Particulas[i].velocidade.x = normal.x * (velBase * 0.3f) + (randomico() - 0.5f) * spread * 2.0f;
            g_Particulas[i].velocidade.y = normal.y * (velBase * 0.3f) + (randomico() - 0.5f) * spread * 2.0f + 0.005f * randomico();
            g_Particulas[i].velocidade.z = normal.z * (velBase * 0.3f) + (randomico() - 0.5f) * spread * 2.0f;

            g_Particulas[i].aceleracao.x = vento[0] * 0.0005f + (randomico() - 0.5f) * 0.0001f;
            g_Particulas[i].aceleracao.y = vento[1] * 0.0005f;
            g_Particulas[i].aceleracao.z = vento[2] * 0.0005f;

            float c = 0.75f + 0.25f * randomico();
            g_Particulas[i].cor[0] = c;
            g_Particulas[i].cor[1] = c;
            g_Particulas[i].cor[2] = c;

            g_Particulas[i].tempoVida = 1.2f + 1.2f * randomico();
        }
    }

    g_Particulas[i].vidaInicial = g_Particulas[i].tempoVida;
    g_Particulas[i].transparencia = 0.0f;
}

// Verifica se a particula morreu, se sim, chama conceber()
void extinguirParticula(int i) {
    if (g_Particulas[i].tempoVida < 0.01f) {
        conceberParticula(i);
    }
}

// Inicializa o sistema de particulas inteiro
void iniciaParticulas(void) {
    for (int i = 0; i < MAXPARTICULAS; i++) {
        conceberParticula(i);
        g_Particulas[i].tempoVida *= randomico();
        g_Particulas[i].vidaInicial = g_Particulas[i].tempoVida > 0.0001f ? g_Particulas[i].tempoVida : 0.5f;
    }
}

// Desenha todas as particulas
void desenhaParticulas() {
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Ajusta transparencia dependendo do tipo
    if (tipoSim == 0) { // fogo é aditivo
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glPointSize(3.0f);
    }
    else { // fumaca e agua usam alfa tradicional
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPointSize(4.0f);
    }

    glBegin(GL_POINTS);
    for (int i = 0; i < MAXPARTICULAS; i++) {
        Particula* p = &g_Particulas[i];
        float fatorIdade = (p->vidaInicial > 0.0001f) ? (p->tempoVida / p->vidaInicial) : 0.0f;
        if (fatorIdade < 0.0f) fatorIdade = 0.0f;
        if (fatorIdade > 1.0f) fatorIdade = 1.0f;


        // Cor/alpha dependendo do tipo e do tempo de vida
        float r = p->cor[0];
        float g = p->cor[1];
        float b = p->cor[2];
        float a = 1.0f;

        if (tipoSim == 0) { // fogo
            float r_final = 0.6f, g_final = 0.12f, b_final = 0.0f;
            r = r_final + (p->cor[0] - r_final) * fatorIdade;
            g = g_final + (p->cor[1] - g_final) * fatorIdade;
            b = b_final + (p->cor[2] - b_final) * fatorIdade;
            a = powf(fatorIdade, 0.4f);
            glPointSize(2.0f + 3.0f * (1.0f - fatorIdade));

        }
        else if (tipoSim == 1) { // fumaca
            a = (1.0f - fabsf(0.5f - fatorIdade) * 2.0f) * 0.6f;
            glPointSize(3.0f + 4.0f * (1.0f - fatorIdade));
            r = p->cor[0] * (0.9f + 0.1f * (1.0f - fatorIdade));
            g = p->cor[1] * (0.9f + 0.1f * (1.0f - fatorIdade));
            b = p->cor[2] * (0.95f + 0.05f * (1.0f - fatorIdade));

        }
        else { // agua
            a = 0.7f * fatorIdade;
            glPointSize(2.0f + 2.0f * fatorIdade);
        }

        p->transparencia = a;
        glColor4f(r, g, b, a);
        glVertex3f(p->posicao.x, p->posicao.y, p->posicao.z);
    }

    glEnd();
}

// Verifica se o ponto está na face, acima da face ou abaixo da face
float distanciaParaFace(tipPto3f ponto, tipPto3f normal, tipPto3f centroFace) {
    // d > 0 : O ponto está acima da face
    // d < 0: O ponto está abaixo da face
    // d == 0:  ponto está exatamente sobre o plano.
    float d = normal.x * (ponto.x - centroFace.x) +
        normal.y * (ponto.y - centroFace.y) +
        normal.z * (ponto.z - centroFace.z);
    return d;
}

// Atualiza a fisica de todas as particulas
void atualizarParticulas(float dt, float elapsedSec) {

    for (int i = 0; i < MAXPARTICULAS; i++) {
        Particula* p = &g_Particulas[i];

        // Turbulencia procedural simples adicionada a aceleracao
        float t = elapsedSec;
        float turbX = 0.0006f * sinf(t * 7.0f + i);
        float turbZ = 0.0006f * cosf(t * 5.0f + i * 1.3f);

        p->velocidade.x += (p->aceleracao.x + turbX) * dt;
        p->velocidade.y += (p->aceleracao.y) * dt;
        p->velocidade.z += (p->aceleracao.z + turbZ) * dt;

        // Resistência do ar
        if (tipoSim == 1) { // fumaca
            p->velocidade.x *= 0.9995f;
            p->velocidade.y *= 0.999f;
            p->velocidade.z *= 0.9995f;
        }
        else if (tipoSim == 2) { // agua
            p->velocidade.x *= 0.998f;
            p->velocidade.y *= 0.998f;
            p->velocidade.z *= 0.998f;
        }
        else { // fogo
            p->velocidade.x *= 0.999f;
            p->velocidade.y *= 0.999f;
            p->velocidade.z *= 0.999f;
        }

        // atualiza posicao
        p->posicao.x += p->velocidade.x * dt;
        p->posicao.y += p->velocidade.y * dt;
        p->posicao.z += p->velocidade.z * dt;

        // Diminui a vida
        p->tempoVida -= dt * 0.6f;

        // Comportamentos especificos
        if (tipoSim == 1) {
            float fatorIdade = 1.0f - ((p->vidaInicial > 0.0001f) ? (p->tempoVida / p->vidaInicial) : 0.0f);
            p->posicao.x += (randomico() - 0.5f) * 0.0008f * fatorIdade;
            p->posicao.z += (randomico() - 0.5f) * 0.0008f * fatorIdade;
        }

        if (tipoSim == 2) {
            tipObjeto* obj = &g_Objetos.vobjs[g_IndiceObjetoAtual];
            tipFace* face = &obj->face[indFaceEmissora];

            // Calcular centro e normal da face para colisão
            tipPto3f centroFace = { 0.0f, 0.0f, 0.0f };
            for (int j = 0; j < face->n; j++) {
                tipPto3f* vert = &obj->vertice->vets[face->indVertice[j]];
                centroFace.x += vert->x;
                centroFace.y += vert->y;
                centroFace.z += vert->z;
            }
            centroFace.x /= face->n;
            centroFace.y /= face->n;
            centroFace.z /= face->n;

            tipPto3f normal;
            calcularNormal(face, obj->vertice, &normal);

            float dist = distanciaParaFace(p->posicao, normal, centroFace);
            float velDotN = p->velocidade.x * normal.x +
                p->velocidade.y * normal.y +
                p->velocidade.z * normal.z;

            // Só verifica colisão se a gota cruzar o plano
            if (dist < 0.0f && velDotN < 0.0f) {

                // Reposiciona levemente acima da face
                p->posicao.x -= normal.x * dist * 1.01f;
                p->posicao.y -= normal.y * dist * 1.01f;
                p->posicao.z -= normal.z * dist * 1.01f;

                // Se a velocidade normal for pequena, a gota nao respinga e evapora
                if (fabsf(velDotN) < 0.0025f) {
                    // diminui a vida suavemente para desaparecer em ~0.3s
                    p->tempoVida -= dt * 3.0f;
                    if (p->tempoVida < 0.0f) p->tempoVida = 0.0f;
                    continue;
                }

                // Deslizamento leve
                p->velocidade.x -= 1.0f * velDotN * normal.x;
                p->velocidade.y -= 1.0f * velDotN * normal.y;
                p->velocidade.z -= 1.0f * velDotN * normal.z;

                // Diminui velocidade e vida, mas mantendo fluidez
                p->velocidade.x *= 0.6f;
                p->velocidade.y *= 0.6f;
                p->velocidade.z *= 0.6f;
                p->tempoVida *= 0.8f;

                // Cria respingos
                if (randomico() < 0.3f) {
                    int nRespingos = 1 + (int)(randomico() * 2);
                    for (int s = 0; s < nRespingos; s++) {
                        int idx = encontrarParticulaLivre();

                        if (idx >= 0) {
                            g_Particulas[idx].posicao = p->posicao;
                            float ang = 2.0f * M_PI * randomico();
                            float mag = 0.015f * (0.5f + randomico() * 0.5f);

                            g_Particulas[idx].velocidade.x = cosf(ang) * mag + normal.x * 0.01f;
                            g_Particulas[idx].velocidade.y = 0.015f + 0.02f * randomico() + normal.y * 0.01f;
                            g_Particulas[idx].velocidade.z = sinf(ang) * mag + normal.z * 0.01f;

                            g_Particulas[idx].aceleracao.x = vento[0] * 0.001f;
                            g_Particulas[idx].aceleracao.y = vento[1] * 0.001f;
                            g_Particulas[idx].aceleracao.z = vento[2] * 0.001f;

                            g_Particulas[idx].cor[0] = 0.05f;
                            g_Particulas[idx].cor[1] = 0.3f;
                            g_Particulas[idx].cor[2] = 0.7f;

                            g_Particulas[idx].tempoVida = 0.4f + 0.3f * randomico();
                            g_Particulas[idx].vidaInicial = g_Particulas[idx].tempoVida;
                            g_Particulas[idx].transparencia = 0.8f;
                        }
                    }
                }
            }
        }

        extinguirParticula(i);
    }
}



void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glPushMatrix();
    glRotatef(g_rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(g_rotY, 0.0f, 1.0f, 0.0f);
    glScalef(g_escala, g_escala, g_escala);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    tipObjeto* obj = &g_Objetos.vobjs[g_IndiceObjetoAtual];

    if (obj->n > 0) {
        aplicarMaterialECor(obj->material, obj->cor);

        for (int i = 0; i < obj->n; i++) {
            tipFace* face = &obj->face[i];
            tipPto3f normal;
            calcularNormal(face, obj->vertice, &normal);
            glNormal3f(normal.x, normal.y, normal.z);

            glBegin(GL_POLYGON);
            for (int j = 0; j < face->n; j++) {
                tipPto3f* vert = &obj->vertice->vets[face->indVertice[j]];
                glVertex3f(vert->x, vert->y, vert->z);
            }
            glEnd();
        }
    }

    // Particulas
    if (g_AnimacaoAtiva && obj->n > 0) {
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);

        desenhaParticulas();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    glPopMatrix();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(45, (float)w / h, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    tipObjeto* obj = &g_Objetos.vobjs[g_IndiceObjetoAtual];

    switch (key) {
    case 'q': case 27: exit(0); break;
    case '1':
        g_IndiceObjetoAtual = 0;
        indFaceEmissora = 0;
        break;
    case '2':
        g_IndiceObjetoAtual = 1;
        indFaceEmissora = 0;
        break;
    case '3':
        g_IndiceObjetoAtual = 2;
        indFaceEmissora = 0;
        break;
    case '4':
        g_IndiceObjetoAtual = 3;
        indFaceEmissora = 0;
        break;

    case '+': case '=':
        g_escala += 0.1f;
        if (g_escala > 5.0f) g_escala = 5.0f;
        printf("Escala: %.1fx\n", g_escala);
        break;
    case '-': case '_':
        g_escala -= 0.1f;
        if (g_escala < 0.1f) g_escala = 0.1f;
        printf("Escala: %.1fx\n", g_escala);
        break;
    case 'r': case 'R':
        g_escala = 1.0f;
        printf("Escala resetada: 1.0x\n");
        break;

    case 'a': case 'A':
        g_AnimacaoAtiva = !g_AnimacaoAtiva;
        printf("Animacao: %s\n", g_AnimacaoAtiva ? "ON" : "OFF");
        break;
    case 'f': case 'F':
        if (obj->n > 0) {
            indFaceEmissora = (indFaceEmissora + 1) % obj->n;
            printf("Trocando emissor para Face %d (de %d)\n", indFaceEmissora, obj->n);
            iniciaParticulas(); // Reinicializa as partículas para a nova face
        }
        break;
    case 's': case 'S':
        tipoSim = (tipoSim + 1) % 3;
        {
            const char* tipos[] = { "Fogo", "Vapor/Fumaca", "Agua" };
            printf("Trocando tipo de simulacao para: %s\n", tipos[tipoSim]);
        }
        iniciaParticulas();
        break;

    case 'x': case 'X':
        raioEmissao += 0.05f;
        if (raioEmissao > 2.0f) raioEmissao = 2.0f;
        printf("Raio do emissor: %.2f\n", raioEmissao);
        break;
    case 'z': case 'Z':
        raioEmissao -= 0.05f;
        if (raioEmissao < 0.05f) raioEmissao = 0.05f;
        printf("Raio do emissor: %.2f\n", raioEmissao);
        break;

    case 'c': case 'C': // Aumentar altura da nuvem
        alturaNuvem += 0.2f;
        if (alturaNuvem > 5.0f) alturaNuvem = 5.0f;
        printf("Altura da nuvem: %.2f\n", alturaNuvem);
        if (tipoSim == 2) iniciaParticulas(); // Reinicializa se estiver em modo chuva
        break;
    case 'v': case 'V': // Diminuir altura da nuvem
        alturaNuvem -= 0.2f;
        if (alturaNuvem < 0.5f) alturaNuvem = 0.5f;
        printf("Altura da nuvem: %.2f\n", alturaNuvem);
        if (tipoSim == 2) iniciaParticulas();
        break;

        // Controles de vento
    case 'j': vento[0] += 3.00f; printf("Vento X: %.2f\n", vento[0]); break;
    case 'h': vento[0] -= 3.00f; printf("Vento X: %.2f\n", vento[0]); break;
    case 'p': vento[1] += 3.00f; printf("Vento Y: %.2f\n", vento[1]); break;
    case 'o': vento[1] -= 3.00f; printf("Vento Y: %.2f\n", vento[1]); break;
    case 'l': vento[2] += 3.00f; printf("Vento Z: %.2f\n", vento[2]); break;
    case 'k': vento[2] -= 3.00f; printf("Vento Z: %.2f\n", vento[2]); break;
    case '0': vento[0] = vento[1] = vento[2] = 0.0f; printf("Vento resetado\n"); break;
    }

    if (!g_AnimacaoAtiva) {
        glutPostRedisplay();
    }
}

void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP: g_rotX -= 5.0f; break;
    case GLUT_KEY_DOWN: g_rotX += 5.0f; break;
    case GLUT_KEY_LEFT: g_rotY -= 5.0f; break;
    case GLUT_KEY_RIGHT: g_rotY += 5.0f; break;
    }
    if (!g_AnimacaoAtiva) {
        glutPostRedisplay();
    }
}

void idle() {
    int atualMs = glutGet(GLUT_ELAPSED_TIME);
    if (ultimoMs == 0) ultimoMs = atualMs;
    int diff = atualMs - ultimoMs;
    if (diff < 0) diff = 0;
    float dt = (float)diff * 0.001f;
    if (dt > 0.05f) dt = 0.05f;

    if (g_AnimacaoAtiva) {
        atualizarParticulas(dt, atualMs * 0.001f);
        glutPostRedisplay();
    }
    ultimoMs = atualMs;
}

void init() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);

    GLfloat global_ambient[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    GLfloat light0_pos[] = { 10.0f, 10.0f, 10.0f, 1.0f };
    GLfloat light0_dif[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light0_amb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light0_spec[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_spec);

    GLfloat light1_pos[] = { -10.0f, 5.0f, -10.0f, 1.0f };
    GLfloat light1_dif[] = { 0.6f, 0.6f, 0.8f, 1.0f };
    GLfloat light1_amb[] = { 0.1f, 0.1f, 0.15f, 1.0f };
    GLfloat light1_spec[] = { 0.6f, 0.6f, 0.8f, 1.0f };
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_amb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_dif);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_spec);

    criarObjetos();
    iniciaParticulas();
}

void printInstrucoes() {
    printf("\n========== CONTROLES DO SISTEMA DE PARTICULAS ==========\n");
    printf("OBJETOS:\n");
    printf("  1-4: Trocar entre objetos (Cubo, Piramide, Prisma, Octaedro)\n");
    printf("  F: Trocar face emissora\n\n");
    printf("ANIMACAO:\n");
    printf("  A: Ligar/Desligar animacao\n");
    printf("  S: Trocar tipo de simulacao (Fogo/Vapor/Agua)\n\n");
    printf("VISUALIZACAO:\n");
    printf("  Setas: Rotacionar objeto\n");
    printf("  +/-: Aumentar/Diminuir escala\n");
    printf("  R: Resetar escala\n\n");
    printf("PARAMETROS DE EMISSAO:\n");
    printf("  X/Z: Aumentar/Diminuir raio do emissor\n");
    printf("  C/V: Aumentar/Diminuir altura da nuvem (modo Agua)\n\n");
    printf("VENTO:\n");
    printf("  H/J: Vento no eixo X\n");
    printf("  O/P: Vento no eixo Y\n");
    printf("  K/L: Vento no eixo Z\n");
    printf("  0: Resetar vento\n\n");
    printf("  Q/ESC: Sair\n");
    printf("========================================================\n\n");
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Sistema de Particulas - Chuva com Nuvem");

    printInstrucoes();

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
