# Panzer-GG — Phase 1: Plugin Base JUCE

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Criar a estrutura base do plugin VST3/Standalone com JUCE 7, parâmetros APVTS completos, processador de áudio funcional (passthrough) e editor com chassis teal mínimo — compilando sem erros no Windows.

**Architecture:** PluginProcessor gerencia a cadeia DSP e o APVTS; PluginEditor renderiza a UI de 860×280px. Fase 1 não implementa DSP real — apenas passthrough + canvas pintado. Todas as dependências entram como git submodules em `extern/`.

**Tech Stack:** JUCE 7, C++17, CMake 3.22+, VST3 SDK (via JUCE), MSVC ou MinGW (Windows)

---

## Scope desta fase

Cobre as tarefas 1–6 do plano oficial (Fase 01 do PDF):
- CMakeLists.txt funcional
- Parameters.h com todos os 20+ parâmetros
- PluginProcessor base (passthrough)
- PluginEditor base (teal 860×280px)
- Build + verificação no AudioPluginHost

Não inclui: DSP real, UI completa, presets, tuner — essas são Fases 2–5.

---

## Mapa de Arquivos

```
panzer-gg/
├── CMakeLists.txt                   ← build system; define plugin, targets, libs
├── extern/
│   └── JUCE/                        ← git submodule (juce-framework/JUCE)
├── src/
│   ├── Parameters.h                 ← createLayout() com todos os parâmetros APVTS
│   ├── PluginProcessor.h            ← declaração AudioProcessor + APVTS
│   ├── PluginProcessor.cpp          ← prepareToPlay, processBlock (passthrough), state
│   ├── PluginEditor.h               ← declaração AudioProcessorEditor
│   └── PluginEditor.cpp             ← paint() com chassis teal mínimo
└── tests/
    └── ParameterTests.cpp           ← testa criação do layout APVTS
```

---

## Task 1: Submodule JUCE + estrutura de diretórios

**Files:**
- Create: `extern/JUCE/` (via git submodule)
- Create: `src/` `tests/` `assets/irs/` `assets/fonts/` `presets/factory/`

- [ ] **Step 1.1: Criar diretórios do projeto**

```bash
cd "/c/Users/cayoc/Desktop/Workspace/Python Projects/Panzer-GG-Multieffects-Pedal"
mkdir -p extern src tests assets/irs assets/fonts presets/factory docs/superpowers/plans
```

- [ ] **Step 1.2: Adicionar JUCE como submodule**

```bash
git submodule add https://github.com/juce-framework/JUCE.git extern/JUCE
git submodule update --init --recursive
```

Esperado: pasta `extern/JUCE/` com conteúdo (CMakeLists.txt, modules/, etc.)

- [ ] **Step 1.3: Verificar versão do JUCE**

```bash
cat extern/JUCE/VERSION
```

Esperado: `7.x.x` (qualquer 7.x serve)

- [ ] **Step 1.4: Commit**

```bash
git add .gitmodules extern/JUCE
git commit -m "feat: add JUCE 7 as git submodule"
```

---

## Task 2: CMakeLists.txt

**Files:**
- Create: `CMakeLists.txt`

- [ ] **Step 2.1: Criar CMakeLists.txt**

Criar o arquivo `CMakeLists.txt` na raiz com o conteúdo:

```cmake
cmake_minimum_required(VERSION 3.22)
project(PanzerGG VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Desabilitar builds de exemplos/extras do JUCE para acelerar compilação
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(extern/JUCE)

juce_add_plugin(PanzerGG
    COMPANY_NAME                "PanzerGG"
    COMPANY_WEBSITE             ""
    COMPANY_EMAIL               ""
    PLUGIN_MANUFACTURER_CODE    "PnGG"
    PLUGIN_CODE                 "PnG1"
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Panzer-GG"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             "Fx" "Distortion"
)

target_sources(PanzerGG
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
)

target_include_directories(PanzerGG
    PRIVATE
        src/
)

target_compile_definitions(PanzerGG
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(PanzerGG
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ── Testes unitários (console app JUCE) ──────────────────────────
juce_add_console_app(PanzerGGTests
    PRODUCT_NAME "PanzerGG Tests"
)

target_sources(PanzerGGTests PRIVATE tests/ParameterTests.cpp)
target_include_directories(PanzerGGTests PRIVATE src/)
target_compile_definitions(PanzerGGTests PRIVATE JUCE_UNIT_TESTS=1)
target_link_libraries(PanzerGGTests PRIVATE juce::juce_audio_processors juce::juce_recommended_warning_flags)
```

- [ ] **Step 2.2: Testar configuração do CMake**

```bash
cd "/c/Users/cayoc/Desktop/Workspace/Python Projects/Panzer-GG-Multieffects-Pedal"
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

Esperado: configuração termina sem erros. Ignorar warnings de deprecação do JUCE.
Se falhar com "JUCE not found": confirmar que `extern/JUCE/CMakeLists.txt` existe.

- [ ] **Step 2.3: Commit**

```bash
git add CMakeLists.txt
git commit -m "feat: add CMakeLists.txt with JUCE plugin target and test app"
```

---

## Task 3: Parameters.h

**Files:**
- Create: `src/Parameters.h`
- Test: `tests/ParameterTests.cpp`

- [ ] **Step 3.1: Escrever o teste primeiro**

Criar `tests/ParameterTests.cpp`:

```cpp
#include <JuceHeader.h>
#include "Parameters.h"

// Teste JUCE nativo — rode com o executável PanzerGGTests
struct ParameterLayoutTest : juce::UnitTest
{
    ParameterLayoutTest() : juce::UnitTest ("ParameterLayout", "PanzerGG") {}

    void runTest() override
    {
        beginTest ("createLayout returns non-empty layout");
        auto layout = PanzerGGParameters::createLayout();

        // Conta parâmetros esperados
        int count = 0;
        for (auto* p : layout.getParameters (false))
        {
            juce::ignoreUnused (p);
            ++count;
        }
        // 15 knobs + 4 operacionais = 19 parâmetros
        expectGreaterOrEqual (count, 19);

        beginTest ("gate param range");
        auto* gate = layout.getParameterWithID ("gate");
        expect (gate != nullptr, "gate parameter must exist");

        beginTest ("ampType param range");
        auto* ampType = layout.getParameterWithID ("ampType");
        expect (ampType != nullptr, "ampType parameter must exist");

        beginTest ("irSlot param range");
        auto* irSlot = layout.getParameterWithID ("irSlot");
        expect (irSlot != nullptr, "irSlot parameter must exist");

        beginTest ("liveMode param exists");
        auto* live = layout.getParameterWithID ("liveMode");
        expect (live != nullptr, "liveMode parameter must exist");

        beginTest ("tuner param exists");
        auto* tuner = layout.getParameterWithID ("tuner");
        expect (tuner != nullptr, "tuner parameter must exist");
    }
};

static ParameterLayoutTest parameterLayoutTest;

int main()
{
    juce::UnitTestRunner runner;
    runner.runAllTests();

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult(i)->failures;

    return failures > 0 ? 1 : 0;
}
```

- [ ] **Step 3.2: Verificar que o teste não compila ainda (sem Parameters.h)**

```bash
cmake --build build --target PanzerGGTests --config Debug 2>&1 | head -20
```

Esperado: erro "Parameters.h: No such file or directory"

- [ ] **Step 3.3: Criar Parameters.h**

Criar `src/Parameters.h`:

```cpp
#pragma once
#include <JuceHeader.h>

namespace PanzerGGParameters
{
    // IDs dos parâmetros — usar estas constantes em todo o código
    namespace ID
    {
        static const juce::String gate       = "gate";
        static const juce::String ampType    = "ampType";
        static const juce::String gain       = "gain";
        static const juce::String treble     = "treble";
        static const juce::String middle     = "middle";
        static const juce::String bass       = "bass";
        static const juce::String ampVol     = "ampVol";
        static const juce::String modFx      = "modFx";
        static const juce::String modSpeed   = "modSpeed";
        static const juce::String dlyMix     = "dlyMix";
        static const juce::String dlyTime    = "dlyTime";
        static const juce::String rvbDecay   = "rvbDecay";
        static const juce::String rvbMix     = "rvbMix";
        static const juce::String irSlot     = "irSlot";
        static const juce::String master     = "master";
        static const juce::String liveMode   = "liveMode";
        static const juce::String bank       = "bank";
        static const juce::String slot       = "slot";
        static const juce::String tuner      = "tuner";
    }

    // Valores padrão
    namespace Default
    {
        constexpr float gate      = -80.f;   // far-left = OFF
        constexpr int   ampType   = 0;
        constexpr float gain      = 50.f;
        constexpr float treble    = 50.f;
        constexpr float middle    = 50.f;
        constexpr float bass      = 50.f;
        constexpr float ampVol    = 70.f;
        constexpr float modFx     = 0.31f;   // zona morta = MOD OFF por padrão
        constexpr float modSpeed  = 50.f;
        constexpr float dlyMix    = 0.31f;   // zona morta = DELAY OFF por padrão
        constexpr float dlyTime   = 350.f;   // ms
        constexpr float rvbDecay  = 0.31f;   // zona morta = REVERB OFF por padrão
        constexpr float rvbMix    = 25.f;
        constexpr int   irSlot    = 0;       // 0 = IR OFF
        constexpr float master    = 75.f;
        constexpr int   bank      = 0;
        constexpr int   slot      = -1;      // -1 = BYPASS total
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using P = juce::AudioParameterFloat;
        using I = juce::AudioParameterInt;
        using B = juce::AudioParameterBool;

        juce::AudioProcessorValueTreeState::ParameterLayout params;

        // ── NOISE GATE ────────────────────────────────────────────
        // far-left (-80dB) = gate fechado (OFF)
        params.add (std::make_unique<P> (
            ID::gate, "Noise Gate",
            juce::NormalisableRange<float> (-80.f, 0.f, 0.1f), Default::gate));

        // ── AMP MODULE ───────────────────────────────────────────
        // 9 tipos (0–8): clean, crunch, lead, etc.
        params.add (std::make_unique<I> (ID::ampType, "AMP Type", 0, 8, Default::ampType));
        params.add (std::make_unique<P> (
            ID::gain, "Gain",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::gain));
        params.add (std::make_unique<P> (
            ID::treble, "Treble",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::treble));
        params.add (std::make_unique<P> (
            ID::middle, "Middle",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::middle));
        params.add (std::make_unique<P> (
            ID::bass, "Bass",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::bass));
        params.add (std::make_unique<P> (
            ID::ampVol, "AMP Volume",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::ampVol));

        // ── MOD — knob de 3 zonas ────────────────────────────────
        // [0.0–0.305] CHORUS mix | [0.31–0.355] OFF | [0.36–0.63] PHASER depth
        // [0.63–0.685] OFF | [0.69–1.0] TREMOLO mix
        params.add (std::make_unique<P> (
            ID::modFx, "MOD FX",
            juce::NormalisableRange<float> (0.f, 1.f, 0.001f), Default::modFx));
        params.add (std::make_unique<P> (
            ID::modSpeed, "MOD Speed",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::modSpeed));

        // ── DELAY — knob de 3 zonas ──────────────────────────────
        // [0.0–0.305] ANALOG | [0.31–0.355] OFF | [0.36–0.63] TAPE
        // [0.63–0.685] OFF | [0.69–1.0] DUAL (ping-pong)
        params.add (std::make_unique<P> (
            ID::dlyMix, "DLY Mix",
            juce::NormalisableRange<float> (0.f, 1.f, 0.001f), Default::dlyMix));
        params.add (std::make_unique<P> (
            ID::dlyTime, "DLY Time",
            juce::NormalisableRange<float> (0.f, 1000.f, 1.f), Default::dlyTime));

        // ── REVERB — knob de 3 zonas ─────────────────────────────
        // [0.0–0.305] ROOM | [0.31–0.355] OFF | [0.36–0.63] SPRING
        // [0.63–0.685] OFF | [0.69–1.0] CLOUD
        params.add (std::make_unique<P> (
            ID::rvbDecay, "RVB Decay",
            juce::NormalisableRange<float> (0.f, 1.f, 0.001f), Default::rvbDecay));
        params.add (std::make_unique<P> (
            ID::rvbMix, "RVB Mix",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::rvbMix));

        // ── IR CAB ───────────────────────────────────────────────
        // 0 = IR OFF, 1–8 = 8 slots de IR
        params.add (std::make_unique<I> (ID::irSlot, "IR CAB", 0, 8, Default::irSlot));

        // ── MASTER ───────────────────────────────────────────────
        params.add (std::make_unique<P> (
            ID::master, "Master",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::master));

        // ── OPERACIONAIS ─────────────────────────────────────────
        params.add (std::make_unique<B>  (ID::liveMode, "Live Mode", false));
        params.add (std::make_unique<I>  (ID::bank,     "Bank",      0, 8, Default::bank));
        // slot: -1 = BYPASS total, 0-3 = presets A/B/C/D
        params.add (std::make_unique<I>  (ID::slot,     "Slot",     -1, 3, Default::slot));
        params.add (std::make_unique<B>  (ID::tuner,    "Tuner",    false));

        return params;
    }

} // namespace PanzerGGParameters
```

- [ ] **Step 3.4: Rodar o teste**

```bash
cmake --build build --target PanzerGGTests --config Debug
./build/PanzerGGTests_artefacts/Debug/PanzerGGTests
```

Esperado: todos os testes passando, saída mostrando "All tests passed".

- [ ] **Step 3.5: Commit**

```bash
git add src/Parameters.h tests/ParameterTests.cpp
git commit -m "feat: add APVTS parameter layout with all 19 parameters"
```

---

## Task 4: PluginProcessor base

**Files:**
- Create: `src/PluginProcessor.h`
- Create: `src/PluginProcessor.cpp`

- [ ] **Step 4.1: Criar PluginProcessor.h**

```cpp
#pragma once
#include <JuceHeader.h>
#include "Parameters.h"

class PanzerGGAudioProcessor : public juce::AudioProcessor
{
public:
    PanzerGGAudioProcessor();
    ~PanzerGGAudioProcessor() override;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==========================================================================
    const juce::String getName() const override { return JucePlugin_Name; }
    bool   acceptsMidi()      const override { return false; }
    bool   producesMidi()     const override { return false; }
    bool   isMidiEffect()     const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; } // reverb tail

    //==========================================================================
    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    juce::AudioProcessorValueTreeState apvts;

private:
    // Fase 1: passthrough puro. DSP real implementado na Fase 2.
    bool isBypassed() const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanzerGGAudioProcessor)
};
```

- [ ] **Step 4.2: Criar PluginProcessor.cpp**

```cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PanzerGGAudioProcessor::PanzerGGAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::mono(),  true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PanzerGGState",
             PanzerGGParameters::createLayout())
{
}

PanzerGGAudioProcessor::~PanzerGGAudioProcessor() {}

//==============================================================================
void PanzerGGAudioProcessor::prepareToPlay (double /*sampleRate*/,
                                            int /*samplesPerBlock*/)
{
    // Fase 1: nada a preparar — passthrough
    // Fase 2: inicializar cadeia DSP aqui
}

void PanzerGGAudioProcessor::releaseResources() {}

bool PanzerGGAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Aceita mono ou stereo na entrada, sempre stereo na saída
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::stereo())
        return false;

    return (in == juce::AudioChannelSet::mono() ||
            in == juce::AudioChannelSet::stereo());
}

//==============================================================================
bool PanzerGGAudioProcessor::isBypassed() const noexcept
{
    // BYPASS total: modo PRESET e nenhum slot ativo (slot == -1)
    const bool isLive = apvts.getRawParameterValue (PanzerGGParameters::ID::liveMode)->load() > 0.5f;
    const int  slot   = static_cast<int> (
        apvts.getRawParameterValue (PanzerGGParameters::ID::slot)->load());

    return (!isLive && slot == -1);
}

void PanzerGGAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    // Aplicar MASTER volume (sempre ativo mesmo no bypass — igual ao hardware)
    const float masterNorm = apvts.getRawParameterValue (
        PanzerGGParameters::ID::master)->load() / 100.f;

    // Tuner: muta saída completamente
    const bool tunerActive = apvts.getRawParameterValue (
        PanzerGGParameters::ID::tuner)->load() > 0.5f;

    if (tunerActive)
    {
        buffer.clear();
        return;
    }

    if (isBypassed())
    {
        // BYPASS: passa sinal sem processamento, mas aplica master
        buffer.applyGain (masterNorm);
        return;
    }

    // Fase 1: passthrough com master volume
    // Fase 2: aqui entrará a cadeia: gate → amp → mod → delay → reverb → ir → master
    buffer.applyGain (masterNorm);
}

//==============================================================================
juce::AudioProcessorEditor* PanzerGGAudioProcessor::createEditor()
{
    return new PanzerGGAudioProcessorEditor (*this);
}

//==============================================================================
void PanzerGGAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PanzerGGAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PanzerGGAudioProcessor();
}
```

- [ ] **Step 4.3: Build para verificar que compila (vai falhar pois Editor ainda não existe)**

```bash
cmake --build build --target PanzerGG_VST3 --config Debug 2>&1 | grep -E "error:|warning:" | head -30
```

Esperado: erros sobre `PanzerGGAudioProcessorEditor` não declarado. Isso é esperado — próxima task resolve.

- [ ] **Step 4.4: Commit parcial**

```bash
git add src/PluginProcessor.h src/PluginProcessor.cpp
git commit -m "feat: add PluginProcessor with APVTS, passthrough processBlock and state serialization"
```

---

## Task 5: PluginEditor base (chassis teal)

**Files:**
- Create: `src/PluginEditor.h`
- Create: `src/PluginEditor.cpp`

- [ ] **Step 5.1: Criar PluginEditor.h**

```cpp
#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PanzerGGAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PanzerGGAudioProcessorEditor (PanzerGGAudioProcessor& p);
    ~PanzerGGAudioProcessorEditor() override;

    //==========================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    // Tamanho canônico do plugin (escalável via setResizable)
    static constexpr int kPluginWidth  = 860;
    static constexpr int kPluginHeight = 280;

private:
    void drawChassis       (juce::Graphics& g);
    void drawTopStrip      (juce::Graphics& g);
    void drawCornerScrews  (juce::Graphics& g);
    void drawScrewAt       (juce::Graphics& g, float cx, float cy, float r);

    PanzerGGAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanzerGGAudioProcessorEditor)
};
```

- [ ] **Step 5.2: Criar PluginEditor.cpp**

```cpp
#include "PluginEditor.h"

//==============================================================================
PanzerGGAudioProcessorEditor::PanzerGGAudioProcessorEditor (PanzerGGAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (kPluginWidth, kPluginHeight);
    setResizable (false, false);
}

PanzerGGAudioProcessorEditor::~PanzerGGAudioProcessorEditor() {}

//==============================================================================
void PanzerGGAudioProcessorEditor::paint (juce::Graphics& g)
{
    drawChassis (g);
    drawTopStrip (g);
    drawCornerScrews (g);
}

void PanzerGGAudioProcessorEditor::resized()
{
    // Task 8 montará o layout completo de componentes aqui
}

//==============================================================================
// Chassis: corpo principal teal com gradiente vertical
// Cor exata extraída do hardware: #3a9080 → #1e5a50
void PanzerGGAudioProcessorEditor::drawChassis (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // Gradiente vertical do chassis
    juce::ColourGradient chassisGrad (
        juce::Colour (0xff3a9080),          // teal claro no topo
        0.f, bounds.getY(),
        juce::Colour (0xff1e5a50),          // teal escuro na base
        0.f, bounds.getBottom(),
        false);

    g.setGradientFill (chassisGrad);
    g.fillRoundedRectangle (bounds, 6.f);

    // Borda sutil para definir o contorno do chassis
    g.setColour (juce::Colour (0xff0a2e28).withAlpha (0.6f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 6.f, 1.f);

    // Textura: linhas verticais muito sutis simulando metal escovado
    g.setColour (juce::Colours::black.withAlpha (0.04f));
    for (float x = 0; x < bounds.getWidth(); x += 4.f)
        g.drawVerticalLine (static_cast<int> (x), bounds.getY(), bounds.getBottom());
}

//==============================================================================
// Top strip: faixa escura no topo com logo "M·VAVE TANK-G"
// Cor: #0e2420 → #0a1e1a
void PanzerGGAudioProcessorEditor::drawTopStrip (juce::Graphics& g)
{
    const float stripH = 28.f;
    const auto  bounds = getLocalBounds().toFloat();

    juce::Rectangle<float> strip (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), stripH);

    // Fundo escuro do top strip
    juce::ColourGradient stripGrad (
        juce::Colour (0xff0e2420), 0.f, strip.getY(),
        juce::Colour (0xff0a1e1a), 0.f, strip.getBottom(),
        false);
    g.setGradientFill (stripGrad);
    g.fillRoundedRectangle (strip.withHeight (stripH + 4.f), 6.f);
    g.fillRect (strip.withTrimmedTop (4.f)); // quadrar a parte de baixo

    // Linha dourada separando top strip do chassis
    g.setColour (juce::Colour (0xffe8a020));
    g.fillRect (bounds.getX(), bounds.getY() + stripH, bounds.getWidth(), 2.f);

    // Logo "M·VAVE  TANK-G"
    g.setColour (juce::Colour (0xffa0c8c0));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 11.f,
                           juce::Font::bold));
    g.drawText ("M\xc2\xb7VAVE", strip.withWidth (80.f).translated (14.f, 0.f),
                juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff78c8b8));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 13.f,
                           juce::Font::bold));
    g.drawText ("TANK-G", strip.withWidth (90.f).translated (70.f, 0.f),
                juce::Justification::centredLeft);

    // Label técnico no canto direito
    g.setColour (juce::Colour (0xff3a7068));
    g.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 8.5f,
                           juce::Font::plain));
    g.drawText ("GUITAR MULTI-FX  |  VST3",
                strip.withTrimmedLeft (bounds.getWidth() - 200.f).reduced (8.f, 0.f),
                juce::Justification::centredRight);
}

//==============================================================================
// Parafusos: 4 cantos, gradiente radial metálico + ranhura diagonal 45°
void PanzerGGAudioProcessorEditor::drawCornerScrews (juce::Graphics& g)
{
    const float r      = 8.f;
    const float margin = 10.f;
    const auto  bounds = getLocalBounds().toFloat();

    drawScrewAt (g, bounds.getX()      + margin, bounds.getY()      + margin, r);
    drawScrewAt (g, bounds.getRight()  - margin, bounds.getY()      + margin, r);
    drawScrewAt (g, bounds.getX()      + margin, bounds.getBottom() - margin, r);
    drawScrewAt (g, bounds.getRight()  - margin, bounds.getBottom() - margin, r);
}

void PanzerGGAudioProcessorEditor::drawScrewAt (juce::Graphics& g,
                                                 float cx, float cy, float r)
{
    juce::Rectangle<float> area (cx - r, cy - r, r * 2.f, r * 2.f);

    // Corpo do parafuso: gradiente radial metálico
    juce::ColourGradient screwGrad (
        juce::Colour (0xff5a7a74), cx - r * 0.3f, cy - r * 0.3f,
        juce::Colour (0xff1a2e2c), cx, cy,
        true);
    g.setGradientFill (screwGrad);
    g.fillEllipse (area);

    // Borda do parafuso
    g.setColour (juce::Colour (0xff0a1e1c));
    g.drawEllipse (area, 1.f);

    // Ranhura diagonal a 45°
    g.setColour (juce::Colour (0xff0d2220));
    const float half = r * 0.65f;
    g.drawLine (cx - half, cy - half, cx + half, cy + half, 1.5f);
}
```

- [ ] **Step 5.3: Build completo**

```bash
cmake --build build --target PanzerGG_VST3 --config Debug
cmake --build build --target PanzerGG_Standalone --config Debug
```

Esperado: compilação bem-sucedida sem erros. Warnings são aceitáveis.

Se falhar com erro de `JucePlugin_Name`: verificar que `CMakeLists.txt` tem `PRODUCT_NAME "Panzer-GG"`.

- [ ] **Step 5.4: Commit**

```bash
git add src/PluginEditor.h src/PluginEditor.cpp
git commit -m "feat: add PluginEditor with teal chassis, top strip, corner screws"
```

---

## Task 6: Verificação no AudioPluginHost

**Files:** nenhum arquivo novo — apenas verificação de funcionamento

- [ ] **Step 6.1: Localizar o Standalone gerado**

```bash
find build -name "*.exe" -path "*Standalone*" 2>/dev/null
```

Esperado: path para `PanzerGG.exe` no Standalone artefato.

- [ ] **Step 6.2: Rodar o Standalone**

Abrir o `.exe` encontrado. Deve aparecer uma janela de 860×280px com:
- Fundo teal com gradiente
- Faixa escura no topo com "M·VAVE" e "TANK-G"
- Linha dourada separando faixa do chassis
- 4 parafusos nos cantos

- [ ] **Step 6.3: Localizar o VST3 gerado**

```bash
find build -name "*.vst3" 2>/dev/null
```

- [ ] **Step 6.4: Copiar VST3 para pasta de plugins do sistema (opcional)**

```bash
# Windows — pasta padrão VST3
cp -r "$(find build -name '*.vst3' 2>/dev/null | head -1)" \
      "$LOCALAPPDATA/Programs/Common/VST3/"
```

- [ ] **Step 6.5: Verificar parâmetros no FL Studio (ou qualquer DAW)**

Abrir FL Studio → Mixer → inserir Panzer-GG na slot → clicar com botão direito → "Browse parameters".
Deve listar todos os 19 parâmetros:
`Noise Gate, AMP Type, Gain, Treble, Middle, Bass, AMP Volume, MOD FX, MOD Speed, DLY Mix, DLY Time, RVB Decay, RVB Mix, IR CAB, Master, Live Mode, Bank, Slot, Tuner`

- [ ] **Step 6.6: Commit final da fase**

```bash
git add .
git commit -m "feat: phase 1 complete — plugin base builds, loads in DAW, all parameters visible"
```

---

## Self-Review

### Cobertura do spec
- [x] CMakeLists.txt — VST3 + Standalone ✓
- [x] Parameters.h — 19 parâmetros (15 knobs + 4 operacionais) ✓
- [x] PluginProcessor — prepareToPlay, processBlock, get/setStateInformation ✓
- [x] PluginEditor — 860×280px, cor teal ✓
- [x] Testes para o layout APVTS ✓
- [ ] Testar no FL Studio → coberto no Step 6.5

### Lacunas identificadas
- A Fase 1 não cobre AU (macOS only — ok para Windows)
- `juce_add_binary_data` para IRs não está no CMakeLists desta fase — será adicionado na Fase 4 (IRLoader)
- chowdsp_utils e RTNeural não estão como submodules ainda — serão adicionados no início da Fase 2

### Verificação de tipos
- `PanzerGGParameters::createLayout()` retorna `juce::AudioProcessorValueTreeState::ParameterLayout` — usado corretamente no construtor do Processor ✓
- `apvts.getRawParameterValue(ID::master)->load()` — correto, retorna `std::atomic<float>*` ✓
- `PanzerGGAudioProcessorEditor` declarado em `PluginEditor.h` e referenciado em `PluginProcessor.cpp` via `#include "PluginEditor.h"` ✓
