#include <FWCore/PluginManager/interface/PluginFactory.h>

#include <cp3_llbb/Framework/interface/GenParticlesProducer.h>
#include <cp3_llbb/Framework/interface/EventProducer.h>
#include <cp3_llbb/Framework/interface/HLTProducer.h>
#include <cp3_llbb/Framework/interface/JetsProducer.h>
#include <cp3_llbb/Framework/interface/FatJetsProducer.h>
#include <cp3_llbb/Framework/interface/METProducer.h>
#include <cp3_llbb/Framework/interface/MuonsProducer.h>
#include <cp3_llbb/Framework/interface/TausProducer.h>
#include <cp3_llbb/Framework/interface/ElectronsProducer.h>
#include <cp3_llbb/Framework/interface/VerticesProducer.h>

DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, GenParticlesProducer, "gen_particles");
DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, EventProducer, "event");
DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, HLTProducer, "hlt");
DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, JetsProducer, "jets");
DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, FatJetsProducer, "fat_jets");
DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, METProducer, "met");
DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, MuonsProducer, "muons");
DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, TausProducer, "taus");
DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, ElectronsProducer, "electrons");
DEFINE_EDM_PLUGIN(ExTreeMakerProducerFactory, VerticesProducer, "vertices");
