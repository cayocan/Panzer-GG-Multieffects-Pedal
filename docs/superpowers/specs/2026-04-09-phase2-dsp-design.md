# Panzer-GG — Phase 2: DSP Real

**Date:** 2026-04-09
**Status:** Approved
**Goal:** Implementar a cadeia de processamento de áudio completa, substituindo o passthrough atual por DSP funcional em todos os 6 módulos.

---

## Decisões de design

| Decisão | Escolha | Razão |
|---------|---------|-------|
| Amp modeling | Waveshaping clássico | Implementável sem dados externos, CPU mínima |
| Oversampling | 2x apenas no AmpProcessor | Elimina aliasing relevante, custo controlado |
| Reverb Cloud | FDN 16 linhas + LFO | Qualidade audível sem custo excessivo |
| IR Cab | `juce::dsp::Convolution` nativa | BinaryData já pronto, zero código extra |

---

## Cadeia de sinal

```
IN → GateProcessor → AmpProcessor (2x OS) → ModProcessor → DelayProcessor → ReverbProcessor → IrCabProcessor → Master gain → OUT
```

Cada módulo implementa a interface `DspModule` existente em `src/dsp/DspModule.h`.
O `PluginProcessor::processBlock()` chama cada módulo em sequência.
Oversampling 2x é **interno** ao `AmpProcessor` — demais módulos rodam em sample rate nativo.

---

## Módulo 1: GateProcessor

**Parâmetro:** `gate` (-80dB a 0dB). `-80dB` = gate desligado.

**Algoritmo:**
- Detector de envelope RMS com janela de 10ms
- Quando `rms < threshold`: fade-out com release de 50ms
- Quando `rms >= threshold`: fade-in com attack de 5ms
- Threshold de -80dB desativa o gate completamente (bypass interno)

**Arquivo:** `src/dsp/GateProcessor.cpp`

---

## Módulo 2: AmpProcessor

**Parâmetros:** `ampType` (0–8), `gain` (0–100), `treble` (0–100), `middle` (0–100), `bass` (0–100), `ampVol` (0–100)

**Pipeline:**
1. `juce::dsp::Oversampling<float>` — fator 2x, Kaiser windowed filter
2. Pre-gain: `gain` mapeado para multiplicador 0.5–30x (log scale)
3. Waveshaper por tipo de amp (ver tabela abaixo)
4. EQ 3 bandas via `juce::dsp::IIR`:
   - Bass: shelving baixo fc=250Hz
   - Mid: peak fc=800Hz, Q=1.0
   - Treble: shelving alto fc=3500Hz
   - Cada parâmetro 0–100 → ganho -12dB a +12dB (50 = 0dB)
5. Volume de saída: `ampVol` 0–100 → 0.0–1.0 linear
6. Down-sample de volta ao sample rate nativo

**Curvas de waveshaping:**

| # | Nome | Função |
|---|------|--------|
| 0 | Clean | `tanh(x * 0.8)` |
| 1 | Crunch | `tanh(x * 2.0) * 0.9 + 0.05 * x^2` (assimétrico leve) |
| 2 | Lead | `tanh(x * 4.0) * sign(x) * min(abs(x * 4), 1)` |
| 3 | British | `tanh(x * 3.0)` com soft-clip positivo em 0.85 |
| 4 | American | `atan(x * 2.5) * (2/π)` |
| 5 | Metal | hard-clip em ±0.6 + pre-boost de mids (+6dB a 900Hz) |
| 6 | Blues | `tanh(x * 1.5) + 0.1 * sin(π * x)` (2ª harmônica) |
| 7 | Jazz | `x / (1 + abs(x * 0.5))` (compressão suave) |
| 8 | Fuzz | `sign(x) * sqrt(abs(x * 3.0))` |

**Arquivo:** `src/dsp/AmpProcessor.cpp`

---

## Módulo 3: ModProcessor

**Parâmetros:** `modFx` (0.0–1.0), `modSpeed` (0–100 → 0.1–8 Hz)

**Zonas do knob `modFx`:**

| Zona | Range | Efeito | Mix |
|------|-------|--------|-----|
| Chorus | 0.000–0.305 | ativo | normalizado 0→100% |
| OFF | 0.306–0.355 | bypass | — |
| Phaser | 0.356–0.630 | ativo | normalizado 0→100% |
| OFF | 0.631–0.685 | bypass | — |
| Tremolo | 0.686–1.000 | ativo | normalizado 0→100% |

**Chorus:** 2 linhas de delay moduladas por LFO senoidal em quadratura (L/R desfasados 90°). Delay base 7ms, modulação ±3ms. Wet = mix normalizado da zona.

**Phaser:** 4 estágios all-pass em série. Frequência de corte modulada por LFO (350–1800Hz). Feedback 60%. Wet = mix normalizado.

**Tremolo:** Multiplicação do buffer por `1.0 - depth * (1 - sin(lfo)) / 2`. Profundidade = mix normalizado da zona.

**Arquivo:** `src/dsp/ModProcessor.cpp`

---

## Módulo 4: DelayProcessor

**Parâmetros:** `dlyMix` (0.0–1.0), `dlyTime` (0–1000ms)

**Zonas do knob `dlyMix`:** mesma estrutura de 5 zonas do MOD.

| Zona | Range | Tipo |
|------|-------|------|
| Analog | 0.000–0.305 | BBD-style |
| OFF | 0.306–0.355 | bypass |
| Tape | 0.356–0.630 | wow/flutter |
| OFF | 0.631–0.685 | bypass |
| Dual | 0.686–1.000 | ping-pong |

**Analog:** Linha de delay circular + lowpass 1 polo (fc ~3kHz) no feedback. Feedback 40%.

**Tape:** Delay com `dlyTime` modulado por LFO lento (0.3Hz, ±1.5ms) + highpass (fc ~120Hz) no feedback. Feedback 45%.

**Dual (ping-pong):** Delay L = `dlyTime`, Delay R = `dlyTime * 0.75`. Feedback cruzado L→R e R→L a 35%.

Mix wet/dry: borda da zona OFF = 0%, extremo = 100%.

**Arquivo:** `src/dsp/DelayProcessor.cpp`

---

## Módulo 5: ReverbProcessor

**Parâmetros:** `rvbDecay` (0.0–1.0), `rvbMix` (0–100)

**Zonas do knob `rvbDecay`:** mesma estrutura de 5 zonas.

| Zona | Range | Tipo |
|------|-------|------|
| Room | 0.000–0.305 | FDN 8 linhas |
| OFF | 0.306–0.355 | bypass |
| Spring | 0.356–0.630 | allpass + FDN 4 |
| OFF | 0.631–0.685 | bypass |
| Cloud | 0.686–1.000 | FDN 16 linhas |

**Room:** FDN 8 linhas com tempos primos (~20–80ms). Decay: posição na zona → 0.3–2.5s. Pre-delay 8ms.

**Spring:** 3 filtros all-pass em série (caráter de mola) + FDN 4 linhas, feedback 0.85. Modulação leve 0.5Hz nos tempos. Decay: 0.5–3.0s.

**Cloud:** FDN 16 linhas (50–300ms) com LFO por linha (0.1–0.3Hz, ±2ms). Decay: 2–15s. Pre-delay 20ms.

`rvbMix` (0–100%) controla wet/dry independentemente da zona.

**Arquivo:** `src/dsp/ReverbProcessor.cpp`

---

## Módulo 6: IrCabProcessor

**Parâmetro:** `irSlot` (0–8)

- `irSlot = 0`: bypass completo (sem convolução)
- `irSlot = 1–8`: carrega IR correspondente do `BinaryData`

**Implementação:**
- `juce::dsp::Convolution` com `LoadingThread::loadImpulseResponse()`
- Troca de IR: crossfade interno de 50ms (evita clique)
- IRs disponíveis: `ir_01_bright`, `ir_02_warm`, `ir_03_midscoop`, `ir_04_vintage`, `ir_05_room`, `ir_06_tight`, `ir_07_heavy`, `ir_08_american`

**Arquivo:** `src/dsp/IrCabProcessor.cpp`

---

## Interface DspModule

Todos os módulos respeitam a interface existente em `src/dsp/DspModule.h`:

```cpp
virtual void prepare(double sampleRate, int samplesPerBlock) = 0;
virtual void process(juce::AudioBuffer<float>& buffer) = 0;
virtual void reset() = 0;
```

Os parâmetros APVTS são lidos via ponteiros atômicos (`getRawParameterValue`) dentro de `process()` para thread-safety.

---

## Integração no PluginProcessor

`PluginProcessor::prepareToPlay()` chama `prepare()` em cada módulo em ordem.
`PluginProcessor::processBlock()` chama `process()` em cada módulo em ordem após o bloco de tuner/bypass.
`PluginProcessor::releaseResources()` chama `reset()` em cada módulo.

---

## Testes

Cada módulo terá um smoke test em `tests/` verificando:
- Compila e `prepare()` não crasha
- `process()` com buffer silencioso não produz NaN/Inf
- `process()` com tom senoidal produz saída não-zero (módulo não está em bypass inesperado)
