//
//  prefetchTest.js
//  scripts/developer/tests/scriptableResource 
//
//  Created by Zach Pomerantz on 4/27/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Preloads textures and unloads them.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

Script.include([
    '../../../developer/utilities/cache/cacheStats.js',
    'lib.js',
], function() {
    prefetch(function(frames) {
        // Delete each texture, so the next garbage collection cycle will release them.

        // Setting frames = null breaks the reference,
        // but will not delete frames from the calling scope.
        // Instead, we must mutate it in-place to free its elements for GC
        // (assuming the elements are not held elsewhere).
        while (frames.length) { frames.pop(); }

        // Alternatively, forcibly release each texture without relying on GC.
        // frames.forEach(function(texture) { texture.release(); });

        Script.requestGarbageCollection();
    });
});

