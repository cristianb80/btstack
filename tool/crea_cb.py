#!/usr/bin/env python

import sys
import uuid


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("passare il file gatt seguito da quello con le callback")
        sys.exit(1)
    else:
        let = []
        scr = []
        ntf = []
        lid = {}

        with open(sys.argv[1], 'rt') as gatt:
            for riga in gatt:
                campi = riga.split(',')
                if campi[0] != 'CHARACTERISTIC':
                    pass
                else:
                    try:
                        id = campi[1].strip().upper()
                        _ = uuid.UUID(id)
                        if id in lid:
                            lid[id] += 1
                        else:
                            lid[id] = 1
                        nome = id + '-' + ('%02d' % lid[id])
                        nome = nome.replace('-', '_')
                        if 'READ' in campi[2]:
                            let.append(nome)
                        if 'WRITE' in campi[2]:
                            scr.append(nome)
                        if 'NOTIFY' in campi[2]:
                            ntf.append(nome)
                    except ValueError as e:
                        pass

        with open(sys.argv[2], 'wt') as c:
            if len(let):
                c.write('uint16_t le_read(uint16_t crt, uint16_t ofs, uint8_t * dati, uint16_t dim)\n')
                c.write('{\n')
                c.write('    uint16_t letti = 0 ;\n')
                c.write('\n')
                c.write('    if (NULL == dati) {\n')
                c.write('        // Restituisco la dimensione di ogni caratteristica\n')
                c.write('        switch (crt) {\n')
                for crt in let:
                    macro = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                    c.write('        case _: letti = d ; break ;\n'.replace('_', macro))
                c.write('        default: assert(false) ; break ;\n')
                c.write('        }\n')
                c.write('    }\n')
                c.write('    else {\n')
                c.write('        // Copio (almeno una parte del)la caratteristica\n')
                c.write('        switch(crt) {\n')
                for crt in let:
                    macro = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                    c.write('        case _: memcpy(dati, x + ofs, dim) ; letti = dim ; break ;\n'.replace('_', macro))
                c.write('        default: assert(false) ; break ;\n')
                c.write('        }\n')
                c.write('    }\n')
                c.write('\n')
                c.write('    return letti ;\n')
                c.write('}\n')

            if len(ntf):
                c.write('\n')
                for crt in ntf:
                    macro = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'
                    c.write('static bool ntf_? = false ;\n'.replace('?', macro))
                c.write('\n')
                
            if len(scr) + len(ntf):
                c.write('int le_write(uint16_t crt, uint16_t ofs, uint8_t * dati, uint16_t dim)\n')
                c.write('{\n')
                c.write('   uint16_t scritti = 0 ;\n')
                c.write('\n')
                c.write('   if (NULL == dati) {\n')
                if len(scr):
                    c.write('       switch (crt) {\n')
                    for crt in scr:
                        macro = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                        c.write('       case ?: scritti = d ; break ;\n'.replace('?', macro))
                    c.write('       default: assert(false) ; break ;\n')
                    c.write('       }\n')
                    c.write('   }\n')
                c.write('   else {\n')
                c.write('       switch (crt) {\n')
                for crt in ntf:
                    macro = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'
                    c.write('       case ?: ntf_? = BTP_le_notify_abil(dati) ; break ;\n'.replace('?', macro))
                for crt in scr:
                    macro = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                    c.write('       case ?: memcpy(x + ofs, dati, dim) ; scritti = dim ; break ;\n'.replace('?', macro))
                c.write('       default: assert(false) ; break ;\n')
                c.write('       }\n')
                c.write('   }\n')
                c.write('\n')
                c.write('   return scritti ;\n')
                c.write('}\n')
