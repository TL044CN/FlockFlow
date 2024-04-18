
pipeline {
    agent none
    stages {
        stage('initialize submodules') {
            agent {
                label 'generic'
            }
            steps {
                script {
                    sh 'git submodule update --init --recursive'
                }
            }
        }
        stage('Build') {
            matrix {
                axes {
                    axis {
                        name 'PLATFORM'
                        values /*'windows',*/ 'linux'/*, 'macos'*/
                    }
                    axis {
                        name 'COMPILER'
                        values 'gcc', 'clang'/*, 'msvc'*/
                    }
                    axis {
                        name 'BUILD_TYPE'
                        values 'Release', 'Debug'/*, 'MinSizeRel', 'RelWithDebInfo'*/
                    }
                }

                stages {
                    stage('Select Agent'){
                        agent {
                            label "${PLATFORM}&&${COMPILER}"
                        }
                        stages {
                            stage('Retrieving Artifacts') {
                                steps {
                                    script{
                                        try {
                                            unstash 'vendor'
                                        } catch(Exception e) {

                                        }
                                        try {
                                            unarchive (mapping: [
                                                "build-${PLATFORM}-${COMPILER}/": "build"
                                            ])
                                            artifactsRetrieved = true
                                        } catch (Exception e) {

                                        }
                                    }
                                }
                            }
                            stage('Prepare Tools'){
                                steps {
                                    script {
                                        if(COMPILER == 'clang') {
                                            sh 'apt install -y llvm'
                                        }
                                        sh '''
                                            apt install -y lcov doxygen python3-venv
                                            python3 -m venv venv
                                            . venv/bin/activate
                                            pip install lcov_cobertura
                                        '''
                                    }
                                }
                            }
                            stage('Build') {
                                steps {
                                    sh """
                                    cmake -B build/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
                                    cmake --build build/ --config=${BUILD_TYPE} -j
                                    """
                                }
                            }
                            stage('Creating Test Coverage Reports') {
                                steps {
                                    catchError(buildResult: 'SUCCESS', stageResult: 'FAILURE') {
                                        sh """
                                        cmake -B build/ -DCMAKE_BUILD_TYPE=Debug
                                        cmake --build build/ --config=${BUILD_TYPE} -j --target coverage

                                        . venv/bin/activate
                                        # Convert lcov report to cobertura format
                                        lcov_cobertura build/coverage.lcov -o coverage.xml
                                        """

                                        recordCoverage(
                                            qualityGates: [
                                                [criticality: 'NOTE', integerThreshold: 60, metric: 'FILE', threshold: 60.0],
                                                [criticality: 'NOTE', integerThreshold: 60, metric: 'LINE', threshold: 60.0],
                                                [criticality: 'NOTE', integerThreshold: 60, metric: 'METHOD', threshold: 60.0]
                                            ],
                                            tools: [[parser: 'COBERTURA', pattern: 'coverage.xml']])
                                    }
                                }
                            }
                            stage('Archiving Artifacts') {
                                steps {
                                    sh 'mv build "build-${PLATFORM}-${COMPILER}"'
                                    archiveArtifacts (artifacts: "build-${PLATFORM}-${COMPILER}/", allowEmptyArchive: true, onlyIfSuccessful: true, fingerprint: true)
                                }
                            }
                        }
                    }
                }
            }
        }
        stage('Static Analysis') {
            agent {
                label 'generic'
            }
            steps {
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                    sh """
                        apt install -y cppcheck cpplint python3-venv
                        python3 -m venv venv
                        . venv/bin/activate
                        pip install lizard
                        cppcheck --enable=all --inconclusive --xml --xml-version=2 -I vendor/FlockFlow/include -I include/ source/* include/* 2> cppcheck.xml
                        cpplint source/* include/* 2> cpplint.txt || true
                        lizard source/* include/* > lizard.out
                    """

                    sh """
python3 -c "
with open('lizard.out', 'r') as infile, open('lizard-gcc.out', 'w') as outfile:
    for line in infile:
        parts = line.split()
        if len(parts) >= 5:
            outfile.write(f'{parts[0]}:{parts[1]}: cyclomatic complexity={parts[3]}, NLOC={parts[2]}\\n')
"
                        cat lizard-gcc.out
                    """

                    recordIssues(
                        sourceCodeRetention: 'LAST_BUILD',
                        tools: [
                            cppCheck(pattern: 'cppcheck.xml'),
                            cppLint(pattern: 'cpplint.txt'),
                            gcc(id: 'lizard', pattern: 'lizard-gcc.out')
                        ]
                    )
                }
            }
        }
    }
}
