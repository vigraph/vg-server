function get_dependency_path(name)
  if name == 'ot-xmlmesh-core' then
    return 'obtools/xmlmesh/core'
  elseif name == 'ot-xmlmesh-send' then
    return 'obtools/xmlmesh/bindings/cli/send'
  elseif name == 'ot-xmlmesh-receive' then
    return 'obtools/xmlmesh/bindings/cli/receive'
  elseif string.sub(name, 1, 11) == 'ot-xmlmesh-' then
    return 'obtools/xmlmesh/' .. string.sub(name, 12)
  elseif name == 'ot-toolgen' then
    return 'obtools/tools/toolgen'
  elseif string.sub(name, 1, 11) == 'ot-obcache-' then
    return 'obtools/obcache/libs/' .. string.sub(name, 12)
  elseif string.sub(name, 1, 3) == 'ot-' then
    return 'obtools/libs/' .. string.sub(name, 4)

  elseif name == 'vg-engine' then
    return 'engine'

  elseif string.sub(name, 1, 23) == 'vg-module-core-control-' then
    return 'modules/core/controls/' .. string.sub(name, 24)
  elseif string.sub(name, 1, 22) == 'vg-module-core-source-' then
    return 'modules/core/sources/' .. string.sub(name, 23)
  elseif string.sub(name, 1, 23) == 'vg-module-core-service-' then
    return 'modules/core/services/' .. string.sub(name, 24)

  elseif string.sub(name, 1, 25) == 'vg-module-vector-control-' then
    return 'modules/vector/controls/' .. string.sub(name, 26)
  elseif string.sub(name, 1, 24) == 'vg-module-vector-source-' then
    return 'modules/vector/sources/' .. string.sub(name, 25)
  elseif string.sub(name, 1, 24) == 'vg-module-vector-filter-' then
    return 'modules/vector/filters/' .. string.sub(name, 25)
  elseif string.sub(name, 1, 25) == 'vg-module-vector-service-' then
    return 'modules/vector/services/' .. string.sub(name, 26)

  elseif string.sub(name, 1, 21) == 'vg-module-ui-control-' then
    return 'modules/ui/controls/' .. string.sub(name, 22)
  elseif string.sub(name, 1, 21) == 'vg-module-ui-service-' then
    return 'modules/ui/services/' .. string.sub(name, 22)

  elseif string.sub(name, 1, 23) == 'vg-module-laser-source-' then
    return 'modules/laser/sources/' .. string.sub(name, 24)
  elseif string.sub(name, 1, 23) == 'vg-module-laser-filter-' then
    return 'modules/laser/filters/' .. string.sub(name, 24)

  elseif string.sub(name, 1, 23) == 'vg-module-midi-control-' then
    return 'modules/midi/controls/' .. string.sub(name, 24)
  elseif string.sub(name, 1, 23) == 'vg-module-midi-service-' then
    return 'modules/midi/services/' .. string.sub(name, 24)

  elseif string.sub(name, 1, 23) == 'vg-module-audio-filter-' then
    return 'modules/audio/filters/' .. string.sub(name, 24)
  elseif string.sub(name, 1, 23) == 'vg-module-audio-source-' then
    return 'modules/audio/sources/' .. string.sub(name, 24)

  elseif string.sub(name, 1, 22) == 'vg-module-dmx-service-' then
    return 'modules/dmx/services/' .. string.sub(name, 23)
  elseif string.sub(name, 1, 22) == 'vg-module-dmx-control-' then
    return 'modules/dmx/controls/' .. string.sub(name, 23)

  elseif string.sub(name, 1, 3) == 'vg-' then
    return 'libs/' .. string.sub(name, 4)
  end
  return nil
end
